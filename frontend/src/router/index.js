// Composables
import { createRouter, createWebHistory } from 'vue-router'

// 导入统一布局组件
import UnifiedLayout from '@/layouts/UnifiedLayout.vue';

const routes = [
  {
    path: '/',
    component: () => import('@/layouts/default/Default.vue'), // 保留首页/登录页的默认布局
    children: [
      {
        path: '',
        name: 'Home',
        component: () => import('@/views/Home.vue'),
      },
    ],
  },
  // 独立的会议室终端路由（不需要登录）
  {
    path: '/room/:roomId',
    name: 'RoomTerminal',
    component: () => import('@/views/room/RoomTerminal.vue'),
    props: true,
    meta: { requiresAuth: false }
  },
  {
    path: '/app',
    component: UnifiedLayout, // 使用统一布局
    children: [
      // 管理端路由
      {
        path: 'admin/users',
        name: 'UserManagement',
        component: () => import('@/views/admin/UserManagement.vue'),
        meta: { requiresAuth: true, roles: ['admin'] }
      },
      {
        path: 'admin/rooms',
        name: 'RoomManagement',
        component: () => import('@/views/admin/RoomManagement.vue'),
        meta: { requiresAuth: true, roles: ['admin'] }
      },

      {
        path: 'admin/meeting-review',
        name: 'MeetingReview',
        component: () => import('@/pages/admin/MeetingReview.vue'),
        meta: { requiresAuth: true, roles: ['admin'] }
      },
      {
        path: 'admin/checkin-management',
        name: 'CheckinManagement',
        component: () => import('@/pages/admin/CheckinManagement.vue'),
        meta: { requiresAuth: true, roles: ['admin'] }
      },
      // 预定端路由
      {
        path: 'booking/query',
        name: 'QueryRoom',
        component: () => import('@/pages/booking/QueryRoom.vue'),
        meta: { requiresAuth: true, roles: ['admin', 'user'] }
      },
      {
        path: 'booking/book/:roomId',
        name: 'BookRoom',
        component: () => import('@/pages/booking/BookRoom.vue'),
        props: true,
        meta: { requiresAuth: true, roles: ['admin', 'user'] }
      },
      {
        path: 'booking/my-bookings',
        name: 'MyBookings',
        component: () => import('@/pages/booking/MyBookings.vue'),
        meta: { requiresAuth: true, roles: ['admin', 'user'] }
      }
    ],
    meta: { requiresAuth: true } // 整个应用区域需要认证
  },
  // 重定向
  {
    path: '/admin',
    redirect: '/app/admin/users'
  },
  {
    path: '/booking',
    redirect: '/app/booking/query'
  }
]

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),
  routes,
})

router.beforeEach((to, from, next) => {
  const requiresAuth = to.matched.some(record => record.meta.requiresAuth);
  const isAuthenticated = !!localStorage.getItem('user-token');
  const userRole = localStorage.getItem('userRole');

  if (requiresAuth) {
    if (!isAuthenticated) {
      // 用户未登录，重定向到首页 (登录页)
      next({ name: 'Home', query: { redirect: to.fullPath } });
    } else {
      // 用户已登录，检查角色权限
      const requiredRoles = to.matched.reduce((acc, record) => {
        if (record.meta.roles) {
          acc.push(...record.meta.roles);
        }
        return acc;
      }, []);

      if (requiredRoles.length > 0 && !requiredRoles.includes(userRole)) {
        // 用户角色不满足路由要求
        const userHomeRoute = userRole === 'admin' ? 'UserManagement' : 'QueryRoom';
        next({ name: userHomeRoute });
      } else {
        // 权限满足，正常导航
        next();
      }
    }
  } else if (to.name === 'Home' && isAuthenticated) {
    // 如果用户已认证但尝试访问首页 (登录页)，则重定向到其角色对应的首页
    if (userRole === 'admin') {
      next({ name: 'UserManagement' });
    } else {
      next({ name: 'QueryRoom' });
    }
  } else {
    // 不需要认证的页面或未登录用户访问登录页，正常导航
    next();
  }
});

export default router
