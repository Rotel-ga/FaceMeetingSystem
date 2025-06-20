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
             <v-btn small color="secondary" @click="openFaceRegisterDialog(item)" class="ml-2">
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
                  <v-stepper-item :complete="faceRegisterStep > 1" :value="1" title="上传图片"></v-stepper-item>
                  <v-divider></v-divider>
                  <v-stepper-item :value="2" title="完成"></v-stepper-item>
                </v-stepper-header>

                <v-stepper-window>
                  <!-- 步骤1: 上传图片 -->
                  <v-stepper-window-item :value="1">
                    <div class="text-center pa-4">
                      <v-icon size="64" color="primary" class="mb-4">mdi-face-recognition</v-icon>
                      <h3 class="mb-4">上传人脸图片</h3>
                      <p class="mb-4">请选择一张清晰的人脸照片进行上传</p>
                      
                      <!-- 文件上传区域 -->
                      <div class="upload-container mb-4">
                        <v-file-input
                          v-model="selectedFile"
                          accept="image/png,image/jpg,image/jpeg,image/bmp"
                          label="选择图片文件"
                          prepend-icon="mdi-camera"
                          show-size
                          @update:model-value="handleFileChange"
                          class="mb-4"
                        ></v-file-input>
                        
                        <!-- 图片预览 -->
                        <div v-if="previewImage" class="image-preview">
                          <v-img
                            :src="previewImage"
                            max-width="300"
                            max-height="300"
                            class="mx-auto mb-4"
                          ></v-img>
                          <v-btn
                            small
                            color="secondary"
                            @click="clearImage"
                          >
                            重新选择
                          </v-btn>
                        </div>
                      </div>
                      
                      <v-alert
                        type="info"
                        text
                        class="mb-4"
                      >
                        支持 PNG、JPG、JPEG、BMP 格式，文件大小不超过 2MB
                      </v-alert>
                    </div>
                  </v-stepper-window-item>

                  <!-- 步骤2: 完成 -->
                  <v-stepper-window-item :value="2">
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
                v-if="faceRegisterStep === 1 && selectedFile" 
                color="primary" 
                @click="submitFaceRegistration"
                :loading="uploading"
                :disabled="!selectedFile || uploading"
              >
                确认注册
              </v-btn>
              
              <v-btn 
                v-if="faceRegisterStep === 2" 
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
import { ref, reactive, computed, onMounted } from 'vue';
import { getUsers, createUser, updateUser, deleteUser, registerFace } from '@/api/userService.js';

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
    const result = await getUsers();
    // 检查响应格式并提取用户数据数组
    const userData = result.success && result.data ? result.data : [];
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

// 人脸注册相关数据
const showFaceRegisterDialog = ref(false);
const selectedUser = ref(null);
const faceRegisterStep = ref(1);
const selectedFile = ref(null);
const previewImage = ref(null);
const uploading = ref(false);

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

const openFaceRegisterDialog = (user) => {
  selectedUser.value = user;
  showFaceRegisterDialog.value = true;
  faceRegisterStep.value = 1;
  selectedFile.value = null;
  previewImage.value = null;
};

const handleFileChange = (file) => {
  if (!file) {
    clearImage();
    return;
  }
  
  // 检查文件类型
  const allowedTypes = ['image/png', 'image/jpg', 'image/jpeg', 'image/bmp'];
  if (!allowedTypes.includes(file.type)) {
    alert('请选择 PNG、JPG、JPEG 或 BMP 格式的图片');
    clearImage();
    return;
  }
  
  // 检查文件大小 (2MB)
  if (file.size > 2 * 1024 * 1024) {
    alert('图片大小不能超过 2MB');
    clearImage();
    return;
  }
  
  selectedFile.value = file;
  
  // 创建预览
  const reader = new FileReader();
  reader.onload = (e) => {
    previewImage.value = e.target.result;
  };
  reader.readAsDataURL(file);
};

const clearImage = () => {
  selectedFile.value = null;
  previewImage.value = null;
};

// 提交人脸注册
const submitFaceRegistration = async () => {
  if (!selectedFile.value) {
    alert('请先选择图片');
    return;
  }
  
  uploading.value = true;
  
  try {
    // 将图片转换为base64
    const base64 = await fileToBase64(selectedFile.value);
    
    // 调用后端API
    const result = await registerFace(selectedUser.value.id, base64);
    
    if (result.success) {
      faceRegisterStep.value = 2;
      alert('人脸注册成功！');
      // 刷新用户列表
      await loadUsers();
    } else {
      alert(result.message || '人脸注册失败');
    }
  } catch (error) {
    console.error('人脸注册失败:', error);
    alert('人脸注册失败: ' + error.message);
  } finally {
    uploading.value = false;
  }
};

const fileToBase64 = (file) => {
  return new Promise((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => {
      // 移除data:image/xxx;base64,前缀
      const base64 = reader.result.split(',')[1];
      resolve(base64);
    };
    reader.onerror = reject;
    reader.readAsDataURL(file);
  });
};

// 关闭人脸注册对话框
const closeFaceRegisterDialog = () => {
  showFaceRegisterDialog.value = false;
  faceRegisterStep.value = 1;
  selectedFile.value = null;
  previewImage.value = null;
  selectedUser.value = null;
  uploading.value = false;
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