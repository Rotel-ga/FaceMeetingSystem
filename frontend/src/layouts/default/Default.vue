<template>
  <v-app>
    <v-navigation-drawer app v-model="drawer">
      <v-list dense>
        <v-list-item link to="/">
          <v-list-item-icon>
            <v-icon>mdi-home</v-icon>
          </v-list-item-icon>
          <v-list-item-title>首页</v-list-item-title>
        </v-list-item>
        <v-list-group v-if="isAdmin" prepend-icon="mdi-account-cog" value="admin">
          <template v-slot:activator="{ props }">
            <v-list-item v-bind="props" title="管理端"></v-list-item>
          </template>
          <v-list-item link to="/admin/users">
            <v-list-item-title>用户管理</v-list-item-title>
            <template v-slot:prepend>
              <v-icon>mdi-account-multiple</v-icon>
            </template>
          </v-list-item>
          <v-list-item link to="/admin/rooms">
            <v-list-item-title>会议室管理</v-list-item-title>
            <template v-slot:prepend>
              <v-icon>mdi-door-closed</v-icon>
            </template>
          </v-list-item>
          <v-list-item link to="/admin/bookings">
            <v-list-item-title>预定管理</v-list-item-title>
            <template v-slot:prepend>
              <v-icon>mdi-calendar-check</v-icon>
            </template>
          </v-list-item>
        </v-list-group>
        <!-- 删除了会议室预定选项 -->
        <!-- 可以为会议室端添加导航 -->
      </v-list>
    </v-navigation-drawer>

    <v-app-bar app>
      <v-app-bar-nav-icon @click.stop="drawer = !drawer"></v-app-bar-nav-icon>
      <v-toolbar-title>人脸识别会议室预定系统</v-toolbar-title>
    </v-app-bar>

    <v-main>
      <v-container fluid>
        <router-view></router-view>
      </v-container>
    </v-main>

    <v-footer app>
      <span>&copy; {{ new Date().getFullYear() }}</span>
    </v-footer>
  </v-app>
</template>

<script setup>
import { ref } from 'vue';

import { computed } from 'vue'; // 引入 computed

const drawer = ref(true); // 默认展开导航栏
const userRole = ref(localStorage.getItem('userRole') || 'user');

// 计算属性，判断当前用户是否为管理员
const isAdmin = computed(() => userRole.value === 'admin');

// 监听 localStorage 中 userRole 的变化，以便在角色切换时（例如重新登录）更新导航
window.addEventListener('storage', (event) => {
  if (event.key === 'userRole') {
    userRole.value = event.newValue || 'user';
  }
});
</script>

<style scoped>
/* 您可以在这里添加一些自定义样式 */
</style>