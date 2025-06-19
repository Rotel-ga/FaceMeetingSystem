<template>
  <v-app>
    <v-navigation-drawer app v-model="drawer" :permanent="$vuetify.display.mdAndUp" expand-on-hover rail>
      <v-list>
        <v-list-item
          prepend-avatar="https://randomuser.me/api/portraits/women/85.jpg" 
          :title="username"
          subtitle="在线"
        ></v-list-item>
      </v-list>

      <v-divider></v-divider>

      <v-list density="compact" nav>
        <v-list-item 
          v-if="userRole === 'admin'" 
          prepend-icon="mdi-shield-account" 
          title="用户管理" 
          value="usermanagement" 
          :to="{ name: 'UserManagement' }"
        ></v-list-item>
        <v-list-item 
          v-if="userRole === 'admin'" 
          prepend-icon="mdi-office-building" 
          title="会议室管理" 
          value="roommanagement" 
          :to="{ name: 'RoomManagement' }"
        ></v-list-item>

        <v-list-item prepend-icon="mdi-magnify" title="查询会议室" value="query" :to="{ name: 'QueryRoom' }"></v-list-item>
        <v-list-item prepend-icon="mdi-calendar-check" title="我的预定" value="mybookings" :to="{ name: 'MyBookings' }"></v-list-item>
        <v-list-item prepend-icon="mdi-logout" title="退出登录" value="logout" @click="handleLogout"></v-list-item>
      </v-list>
    </v-navigation-drawer>

    <v-app-bar app color="primary">
      <v-app-bar-nav-icon @click="drawer = !drawer"></v-app-bar-nav-icon>
      <v-toolbar-title>会议室预定系统</v-toolbar-title>
    </v-app-bar>

    <v-main>
      <v-container fluid>
        <router-view v-slot="{ Component }">
          <v-fade-transition mode="out-in">
            <component :is="Component" />
          </v-fade-transition>
        </router-view>
      </v-container>
    </v-main>

    <AppFooter />
  </v-app>
</template>

<script setup>
import { ref, onMounted } from 'vue';
import { useRouter } from 'vue-router';
import AppFooter from '@/components/AppFooter.vue'; // 假设页脚组件已存在

const drawer = ref(null);
const router = useRouter();
const username = ref('');
const userRole = ref('');

onMounted(() => {
  username.value = localStorage.getItem('username') || '用户';
  userRole.value = localStorage.getItem('userRole') || 'user'; // 默认为普通用户
  // 登录状态检查已由路由守卫处理，此处无需重复
  // if (!localStorage.getItem('user-token')) {
  //   router.push({ name: 'BookingLogin' });
  // }
});

const handleLogout = () => {
  localStorage.removeItem('user-token');
  localStorage.removeItem('username');
  localStorage.removeItem('userRole'); // 退出时清除角色信息
  router.push({ name: 'BookingLogin' });
};

</script>

<style scoped>
/* Styles for the layout */
</style>