<template>
  <v-app>
    <!-- 侧边导航栏 -->
    <v-navigation-drawer
      v-model="drawer"
      :rail="rail"
      permanent
      @click="rail = false"
      color="primary"
      class="unified-drawer"
    >
      <v-list>
        <v-list-item
          prepend-avatar="https://randomuser.me/api/portraits/women/85.jpg" 
          :title="username"
          subtitle="在线"
        ></v-list-item>
      </v-list>

      <v-divider></v-divider>

      <v-list density="compact" nav>
        <!-- 管理员菜单 -->
        <v-list-group v-if="isAdmin" value="admin">
          <template v-slot:activator="{ props }">
            <v-list-item
              v-bind="props"
              prepend-icon="mdi-shield-account"
              title="管理端"
            ></v-list-item>
          </template>
          
          <v-list-item
            prepend-icon="mdi-account-multiple"
            title="用户管理"
            value="usermanagement"
            :to="{ name: 'UserManagement' }"
          ></v-list-item>
          
          <v-list-item
            prepend-icon="mdi-office-building"
            title="会议室管理"
            value="roommanagement"
            :to="{ name: 'RoomManagement' }"
          ></v-list-item>
          
          <v-list-item
            prepend-icon="mdi-calendar-multiple-check"
            title="预定管理"
            value="bookingmanagement"
            :to="{ name: 'BookingManagement' }"
          ></v-list-item>
        </v-list-group>

        <!-- 所有用户都可见的菜单 -->
        <v-list-item 
          prepend-icon="mdi-magnify" 
          title="查询会议室" 
          value="query" 
          :to="{ name: 'QueryRoom' }"
        ></v-list-item>
        
        <v-list-item 
          prepend-icon="mdi-calendar-check" 
          title="我的预定" 
          value="mybookings" 
          :to="{ name: 'MyBookings' }"
        ></v-list-item>
        
        <v-list-item 
          prepend-icon="mdi-logout" 
          title="退出登录" 
          value="logout" 
          @click="handleLogout"
        ></v-list-item>
      </v-list>

      <template v-slot:append>
        <div class="pa-2">
          <v-btn
            block
            @click.stop="rail = !rail"
          >
            <v-icon>{{ rail ? 'mdi-chevron-right' : 'mdi-chevron-left' }}</v-icon>
          </v-btn>
        </div>
      </template>
    </v-navigation-drawer>

    <!-- 顶部应用栏 -->
    <v-app-bar
      elevation="1"
      color="primary"
    >
      <v-app-bar-nav-icon @click.stop="drawer = !drawer" v-if="!drawer"></v-app-bar-nav-icon>
      <v-toolbar-title>人脸识别会议室预定系统</v-toolbar-title>
      <v-spacer></v-spacer>
      <v-chip class="ma-2" color="primary" variant="outlined">
        <v-avatar start>
          <v-icon>mdi-account</v-icon>
        </v-avatar>
        {{ username }}
      </v-chip>
    </v-app-bar>

    <!-- 主内容区域 -->
    <v-main>
      <v-container fluid>
        <router-view v-slot="{ Component }">
          <v-fade-transition mode="out-in">
            <component :is="Component" />
          </v-fade-transition>
        </router-view>
      </v-container>
    </v-main>

    <!-- 页脚 -->
    <v-footer app class="d-flex justify-center align-center" color="primary" height="40">
      <span>&copy; {{ new Date().getFullYear() }} 人脸识别会议室预定系统</span>
    </v-footer>
  </v-app>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue';
import { useRouter } from 'vue-router';

const drawer = ref(true);
const rail = ref(false);
const router = useRouter();
const username = ref('');

// 从localStorage获取用户信息
onMounted(() => {
  username.value = localStorage.getItem('userName') || '用户';
});

// 计算属性，判断当前用户是否为管理员
const isAdmin = computed(() => {
  return localStorage.getItem('userRole') === 'admin';
});

// 退出登录
const handleLogout = () => {
  localStorage.removeItem('user-token');
  localStorage.removeItem('userName');
  localStorage.removeItem('userRole');
  router.push({ name: 'Home' });
};
</script>

<style scoped>
.unified-drawer {
  border-right: 1px solid rgba(255, 255, 255, 0.12);
}
</style>