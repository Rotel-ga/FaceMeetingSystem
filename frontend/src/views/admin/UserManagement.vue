<template>
  <v-container>
    <v-card>
      <v-card-title>用户管理</v-card-title>
      <v-card-text>
        <!-- 用户列表 -->
        <v-data-table
          :headers="userHeaders"
          :items="users"
          class="elevation-1"
        >
          <template v-slot:top>
            <v-toolbar flat>
              <v-toolbar-title>用户列表</v-toolbar-title>
              <v-divider class="mx-4" inset vertical></v-divider>
              <v-spacer></v-spacer>
              <v-btn color="primary" dark class="mb-2" @click="showAddUserDialog = true">
                添加用户
              </v-btn>
            </v-toolbar>
          </template>
          <template v-slot:item.actions="{ item }">
            <v-icon small class="mr-2" @click="editUser(item)">
              mdi-pencil
            </v-icon>
            <v-icon small @click="deleteUserAction(item)">
              mdi-delete
            </v-icon>
             <v-btn small color="secondary" @click="registerFace(item)" class="ml-2">
              注册人脸
            </v-btn>
          </template>
        </v-data-table>

        <!-- 添加/编辑用户对话框 -->
        <v-dialog v-model="showAddUserDialog" max-width="500px">
          <v-card>
            <v-card-title>
              <span class="text-h5">{{ editingUser ? '编辑用户' : '添加用户' }}</span>
            </v-card-title>
            <v-card-text>
              <v-container>
                <v-row>
                  <v-col cols="12">
                    <v-text-field v-model="currentUser.name" label="用户名"></v-text-field>
                  </v-col>
                  <v-col cols="12">
                    <v-text-field v-model="currentUser.password" label="密码" type="password"></v-text-field>
                  </v-col>
                  <v-col cols="12">
                    <v-select
                      v-model="currentUser.role"
                      :items="['admin', 'user']"
                      label="角色"
                    ></v-select>
                  </v-col>
                </v-row>
              </v-container>
            </v-card-text>
            <v-card-actions>
              <v-spacer></v-spacer>
              <v-btn color="blue darken-1" text @click="closeAddUserDialog">取消</v-btn>
              <v-btn color="blue darken-1" text @click="saveUser">保存</v-btn>
            </v-card-actions>
          </v-card>
        </v-dialog>

        <!-- 人脸注册对话框 -->
        <v-dialog v-model="showFaceRegisterDialog" max-width="600px" persistent>
          <v-card>
            <v-card-title>
              <span class="text-h5">为 {{ selectedUser?.name }} 注册人脸</span>
            </v-card-title>
            
            <v-card-text>
              <v-stepper v-model="faceRegisterStep" alt-labels>
                <v-stepper-header>
                  <v-stepper-item :complete="faceRegisterStep > 1" :value="1" title="准备"></v-stepper-item>
                  <v-divider></v-divider>
                  <v-stepper-item :complete="faceRegisterStep > 2" :value="2" title="拍照"></v-stepper-item>
                  <v-divider></v-divider>
                  <v-stepper-item :value="3" title="完成"></v-stepper-item>
                </v-stepper-header>

                <v-stepper-window>
                  <!-- 步骤1: 说明 -->
                  <v-stepper-window-item :value="1">
                    <div class="text-center pa-4">
                      <v-icon size="64" color="primary" class="mb-4">mdi-face-recognition</v-icon>
                      <h3 class="mb-4">人脸注册说明</h3>
                      <v-list>
                        <v-list-item>
                          <v-list-item-icon><v-icon>mdi-numeric-1-circle</v-icon></v-list-item-icon>
                          <v-list-item-content>
                            <v-list-item-title>请确保光线充足</v-list-item-title>
                          </v-list-item-content>
                        </v-list-item>
                        <v-list-item>
                          <v-list-item-icon><v-icon>mdi-numeric-2-circle</v-icon></v-list-item-icon>
                          <v-list-item-content>
                            <v-list-item-title>正面面对摄像头</v-list-item-title>
                          </v-list-item-content>
                        </v-list-item>
                        <v-list-item>
                          <v-list-item-icon><v-icon>mdi-numeric-3-circle</v-icon></v-list-item-icon>
                          <v-list-item-content>
                            <v-list-item-title>需要拍摄3张不同角度的照片</v-list-item-title>
                          </v-list-item-content>
                        </v-list-item>
                      </v-list>
                    </div>
                  </v-stepper-window-item>

                  <!-- 步骤2: 拍照 -->
                  <v-stepper-window-item :value="2">
                    <div class="text-center pa-4">
                      <div class="mb-4">
                        <h4>已拍摄: {{ capturedImages.length }}/3 张照片</h4>
                      </div>
                      
                      <!-- 摄像头预览 -->
                      <div class="camera-container mb-4">
                        <video 
                          ref="faceVideoElement" 
                          width="400" 
                          height="300" 
                          autoplay 
                          muted
                          class="face-camera-preview"
                        ></video>
                        <canvas 
                          ref="faceCanvasElement" 
                          width="400" 
                          height="300" 
                          style="display: none;"
                        ></canvas>
                      </div>

                      <!-- 已拍摄的照片预览 -->
                      <div v-if="capturedImages.length > 0" class="mb-4">
                        <h5>已拍摄照片:</h5>
                        <v-row justify="center">
                          <v-col v-for="(image, index) in capturedImages" :key="index" cols="auto">
                            <v-img :src="image" width="100" height="75" class="ma-1"></v-img>
                          </v-col>
                        </v-row>
                      </div>

                      <!-- 拍照按钮 -->
                      <v-btn 
                        v-if="capturedImages.length < 3" 
                        color="primary" 
                        large 
                        @click="capturePhoto"
                        :disabled="isCapturing"
                      >
                        <v-icon left>mdi-camera</v-icon>
                        拍照 ({{ capturedImages.length + 1 }}/3)
                      </v-btn>
                    </div>
                  </v-stepper-window-item>

                  <!-- 步骤3: 完成 -->
                  <v-stepper-window-item :value="3">
                    <div class="text-center pa-4">
                      <v-icon size="64" color="green" class="mb-4">mdi-check-circle</v-icon>
                      <h3 class="mb-4">人脸注册完成！</h3>
                      <p>已为用户 <strong>{{ selectedUser?.name }}</strong> 成功注册人脸信息。</p>
                      <p class="text-caption">用户现在可以使用人脸识别进行会议室签到。</p>
                    </div>
                  </v-stepper-window-item>
                </v-stepper-window>
              </v-stepper>
            </v-card-text>
            
            <v-card-actions>
              <v-spacer></v-spacer>
              <v-btn 
                v-if="faceRegisterStep === 1" 
                color="blue darken-1" 
                text 
                @click="closeFaceRegisterDialog"
              >
                取消
              </v-btn>
              <v-btn 
                v-if="faceRegisterStep === 1" 
                color="primary" 
                @click="startFaceCapture"
              >
                开始拍照
              </v-btn>
              
              <v-btn 
                v-if="faceRegisterStep === 2" 
                color="blue darken-1" 
                text 
                @click="faceRegisterStep = 1"
              >
                返回
              </v-btn>
              <v-btn 
                v-if="faceRegisterStep === 2 && capturedImages.length === 3" 
                color="primary" 
                @click="submitFaceRegistration"
              >
                提交注册
              </v-btn>
              
              <v-btn 
                v-if="faceRegisterStep === 3" 
                color="primary" 
                @click="closeFaceRegisterDialog"
              >
                完成
              </v-btn>
            </v-card-actions>
          </v-card>
        </v-dialog>
      </v-card-text>
    </v-card>
  </v-container>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue';
import { getUsers, createUser, updateUser, deleteUser } from '@/api/userService.js';

const userHeaders = ref([
  { title: 'ID', value: 'id', key: 'id' },
  { title: '用户名', value: 'name', key: 'name' },
  { title: '密码', value: 'password', key: 'password' },
  { title: '角色', value: 'role', key: 'role' },
  { title: '操作', value: 'actions', sortable: false, key: 'actions' },
]);

const users = ref([]);

const loadUsers = async () => {
  try {
    const userData = await getUsers();
    users.value = userData.map(user => ({
      id: user.id,
      name: user.username,
      password: '******', // 不显示真实密码
      role: user.username === 'admin' ? 'admin' : 'user', // 简单的角色判断
      face_token: user.face_token
    }));
  } catch (error) {
    console.error('获取用户列表失败:', error);
    alert('获取用户列表失败: ' + error.message);
  }
};

onMounted(() => {
  loadUsers();
});

const showAddUserDialog = ref(false);
const editingUser = ref(null);
const currentUser = reactive({
  id: null,
  name: '',
  password: '',
  role: 'user',
});

const editUser = (user) => {
  editingUser.value = user;
  Object.assign(currentUser, user);
  showAddUserDialog.value = true;
};

const deleteUserAction = async (user) => {
  if (confirm(`确定要删除用户 ${user.name} 吗？`)) {
    try {
      await deleteUser(user.id);
      users.value = users.value.filter(u => u.id !== user.id);
      console.log('用户删除成功');
    } catch (error) {
      alert('删除失败: ' + error.message);
    }
  }
};

// 人脸注册相关数据
const showFaceRegisterDialog = ref(false);
const selectedUser = ref(null);
const faceRegisterStep = ref(1);
const capturedImages = ref([]);
const isCapturing = ref(false);
const faceVideoElement = ref(null);
const faceCanvasElement = ref(null);

const registerFace = (user) => {
  selectedUser.value = user;
  showFaceRegisterDialog.value = true;
};

// 开始人脸拍摄
const startFaceCapture = async () => {
  try {
    const stream = await navigator.mediaDevices.getUserMedia({ 
      video: { width: 400, height: 300, facingMode: 'user' } 
    });
    
    if (faceVideoElement.value) {
      faceVideoElement.value.srcObject = stream;
      faceRegisterStep.value = 2;
    }
  } catch (error) {
    console.error('摄像头启动失败:', error);
    alert('摄像头启动失败，请检查设备权限');
  }
};

// 拍照
const capturePhoto = () => {
  if (!faceVideoElement.value || !faceCanvasElement.value) return;
  
  isCapturing.value = true;
  
  const canvas = faceCanvasElement.value;
  const video = faceVideoElement.value;
  const ctx = canvas.getContext('2d');
  
  ctx.drawImage(video, 0, 0, 400, 300);
  const imageData = canvas.toDataURL('image/jpeg', 0.8);
  
  capturedImages.value.push(imageData);
  
  if (capturedImages.value.length === 3) {
    setTimeout(() => {
      isCapturing.value = false;
    }, 500);
  } else {
    isCapturing.value = false;
  }
};

// 提交人脸注册
const submitFaceRegistration = async () => {
  try {
    const registrationData = {
      userId: selectedUser.value.id,
      userName: selectedUser.value.name,
      faceImages: capturedImages.value
    };
    
    const result = await mockFaceRegistrationAPI(registrationData);
    
    if (result.success) {
      const userIndex = users.value.findIndex(u => u.id === selectedUser.value.id);
      if (userIndex !== -1) {
        users.value[userIndex].faceRegistered = true;
        users.value[userIndex].faceId = result.faceId;
        localStorage.setItem('users_data', JSON.stringify(users.value));
      }
      
      faceRegisterStep.value = 3;
    } else {
      alert('人脸注册失败: ' + result.message);
    }
  } catch (error) {
    console.error('人脸注册错误:', error);
    alert('人脸注册过程中发生错误');
  }
};

// 模拟人脸注册API
const mockFaceRegistrationAPI = async (data) => {
  await new Promise(resolve => setTimeout(resolve, 2000));
  
  return {
    success: true,
    faceId: 'face_' + Date.now(),
    message: '人脸注册成功'
  };
};

// 关闭人脸注册对话框
const closeFaceRegisterDialog = () => {
  showFaceRegisterDialog.value = false;
  faceRegisterStep.value = 1;
  capturedImages.value = [];
  selectedUser.value = null;
  
  if (faceVideoElement.value && faceVideoElement.value.srcObject) {
    const tracks = faceVideoElement.value.srcObject.getTracks();
    tracks.forEach(track => track.stop());
  }
};

const closeAddUserDialog = () => {
  showAddUserDialog.value = false;
  editingUser.value = null;
  currentUser.id = null;
  currentUser.name = '';
  currentUser.password = '';
  currentUser.role = 'user';
};

const saveUser = async () => {
  try {
    if (editingUser.value) {
      // 编辑用户
      const updatedUser = await updateUser(currentUser.id, currentUser.name, currentUser.password);
      const index = users.value.findIndex(u => u.id === currentUser.id);
      if (index !== -1) {
        users.value[index] = {
          id: updatedUser.id,
          name: updatedUser.username,
          password: '******',
          role: updatedUser.username === 'admin' ? 'admin' : 'user',
          face_token: updatedUser.face_token
        };
      }
      console.log('用户更新成功');
    } else {
      // 添加新用户
      const newUser = await createUser(currentUser.name, currentUser.password);
      users.value.push({
        id: newUser.id,
        name: newUser.username,
        password: '******',
        role: newUser.username === 'admin' ? 'admin' : 'user',
        face_token: newUser.face_token
      });
      console.log('用户添加成功');
    }
    closeAddUserDialog();
  } catch (error) {
    alert('保存用户失败: ' + error.message);
  }
};
</script>

<style scoped>
.camera-container {
  border: 3px solid #1976d2;
  border-radius: 8px;
  overflow: hidden;
  background: #000;
  display: inline-block;
}

.face-camera-preview {
  display: block;
  border-radius: 4px;
}
</style>