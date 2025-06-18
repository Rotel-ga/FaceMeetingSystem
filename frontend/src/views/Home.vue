<template>
  <v-container fluid fill-height>
    <v-row align="center" justify="center">
      <v-col cols="12" sm="8" md="6" lg="4">
        <v-card class="elevation-12">
          <v-toolbar color="primary" dark flat>
            <v-toolbar-title>欢迎使用人脸识别会议室预定系统</v-toolbar-title>
          </v-toolbar>
          <v-card-text>
            <div class="text-center mb-4">
              <v-img :src="logoUrl" contain height="100" class="my-3"></v-img>
            </div>
            <v-form @submit.prevent="handleLogin">
              <v-text-field
                v-model="username"
                label="用户名"
                name="login"
                prepend-icon="mdi-account"
                type="text"
                required
                variant="underlined"
              ></v-text-field>
              <v-text-field
                v-model="password"
                label="密码"
                name="password"
                prepend-icon="mdi-lock"
                type="password"
                required
                variant="underlined"
              ></v-text-field>
              <v-alert v-if="error" type="error" dense class="mt-3">
                {{ error }}
              </v-alert>
            </v-form>
          </v-card-text>
          <v-card-actions class="pa-4">
            <v-spacer></v-spacer>
            <v-btn color="primary" @click="handleLogin" block large :loading="loading">
              登录
            </v-btn>
            <v-btn v-if="isLoggedIn" color="error" @click="logout" block large class="mt-3">
              登出
            </v-btn>
          </v-card-actions>
        </v-card>
        
        <!-- 会议室终端访问区域 -->
        <v-card class="elevation-8 mt-4">
          <v-card-title class="text-center">
            <v-icon large class="mr-2">mdi-door-open</v-icon>
            会议室终端
          </v-card-title>
          <v-card-text>
            <div class="text-center mb-3">
              <p class="text-body-1">直接访问会议室终端界面，无需登录</p>
            </div>
            <v-row>
              <v-col cols="12" md="6">
                <v-text-field
                  v-model="roomId"
                  label="会议室ID"
                  prepend-icon="mdi-door-closed"
                  type="number"
                  min="1"
                  variant="outlined"
                  hint="请输入要访问的会议室ID"
                  persistent-hint
                ></v-text-field>
              </v-col>
              <v-col cols="12" md="6" class="d-flex align-center">
                <v-btn 
                  color="success" 
                  @click="openRoomTerminal" 
                  block 
                  large
                  :disabled="!roomId || roomId < 1"
                >
                  <v-icon left>mdi-tablet</v-icon>
                  打开会议室终端
                </v-btn>
              </v-col>
            </v-row>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>
  </v-container>
</template>

<script setup>
import { ref } from 'vue';
import { useRouter } from 'vue-router';
import { loginUser } from '@/api/userService.js';
import logoUrl from '@/assets/logo.svg';

const username = ref('');
const password = ref('');
const error = ref('');
const loading = ref(false);
const roomId = ref(''); // 新增：会议室ID
const router = useRouter();
const isLoggedIn = ref(!!localStorage.getItem('user-token'));

// 新增：打开会议室终端的方法
const openRoomTerminal = () => {
  if (roomId.value && roomId.value > 0) {
    // 直接跳转到会议室终端页面
    router.push({ name: 'RoomTerminal', params: { roomId: roomId.value } });
  }
};

const logout = () => {
  localStorage.removeItem('user-token');
  localStorage.removeItem('userName');
  localStorage.removeItem('userRole');
  isLoggedIn.value = false;
  // router.push({ name: 'Home' }); // 当前已在Home页，无需跳转
};

const handleLogin = async () => {
  loading.value = true;
  error.value = '';

  try {
    const response = await loginUser(username.value, password.value);
    
    // 检查响应格式
    if (!response.success || !response.data) {
      throw new Error(response.message || '登录响应格式错误');
    }
    
    const userData = response.data;
    
    // 存储用户信息到localStorage
    localStorage.setItem('user-token', userData.face_token || `token-${userData.username}`);
    localStorage.setItem('userName', userData.username);
    localStorage.setItem('userRole', userData.role);
    localStorage.setItem('userId', userData.id.toString());
    
    isLoggedIn.value = true;
    
    // 根据用户角色跳转到相应页面
    if (userData.role === 'admin') {
      router.push({ name: 'UserManagement' });
    } else {
      router.push({ name: 'QueryRoom' });
    }
  } catch (err) {
    error.value = err.message || '网络连接失败，请检查后端服务是否正常运行';
  }
  
  loading.value = false;
}; // 这是 handleLogin 函数的正确结束

// const anotherFunction = () => { ... }; // 如果有其他函数，也应正确定义和闭合

</script>

<style scoped>
.fill-height {
  min-height: 100vh;
}
</style>