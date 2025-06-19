<template>
  <v-app>
    <!-- 全屏显示，适合iPad/平板 -->
    <v-main class="room-terminal">
      <v-container fluid class="pa-0 fill-height">
        <v-row no-gutters class="fill-height">
          <!-- 左侧：会议室信息和今日安排 -->
          <v-col cols="8" class="pa-4">
            <v-card class="fill-height" elevation="2">
              <v-card-title class="text-h4 primary white--text">
                <v-icon large class="mr-3">mdi-door-open</v-icon>
                {{ roomInfo.name }}
              </v-card-title>
              
              <v-card-subtitle class="text-h6 mt-2">
                {{ roomInfo.location }} | 容量: {{ roomInfo.capacity }}人
              </v-card-subtitle>

              <v-card-text>
                <div class="text-h5 mb-4">今日会议安排</div>
                
                <!-- 当前时间显示 -->
                <div class="text-h6 mb-4 text-center">
                  <v-chip color="primary" large>
                    {{ currentTime }}
                  </v-chip>
                </div>

                <!-- 会议列表 -->
                <v-timeline v-if="todayMeetings.length > 0" dense>
                  <v-timeline-item
                    v-for="meeting in todayMeetings"
                    :key="meeting.id"
                    :color="getMeetingStatus(meeting) === 'current' ? 'green' : 
                            getMeetingStatus(meeting) === 'upcoming' ? 'orange' : 'grey'"
                    small
                  >
                    <template v-slot:opposite>
                      <span class="text-h6">{{ meeting.timeSlots }}</span>
                    </template>
                    <v-card :color="getMeetingStatus(meeting) === 'current' ? 'green lighten-4' : ''">
                      <v-card-title class="text-h6">{{ meeting.reason }}</v-card-title>
                      <v-card-subtitle>预定人: {{ meeting.user }}</v-card-subtitle>
                      <v-card-text v-if="getMeetingStatus(meeting) === 'current'">
                        <v-chip color="green" small>进行中</v-chip>
                      </v-card-text>
                    </v-card>
                  </v-timeline-item>
                </v-timeline>

                <v-alert v-else type="info" class="mt-4">
                  今日暂无会议安排
                </v-alert>
              </v-card-text>
            </v-card>
          </v-col>

          <!-- 右侧：人脸识别区域 -->
          <v-col cols="4" class="pa-4">
            <v-card class="fill-height text-center" elevation="2">
              <v-card-title class="text-h5 justify-center">
                <v-icon large class="mr-2">mdi-face-recognition</v-icon>
                人脸识别签到
              </v-card-title>

              <v-card-text class="d-flex flex-column align-center justify-center fill-height">
                <!-- 摄像头预览区域 -->
                <div class="camera-container mb-4">
                  <video 
                    ref="videoElement" 
                    :width="cameraWidth" 
                    :height="cameraHeight"
                    autoplay 
                    muted
                    class="camera-preview"
                  ></video>
                  <canvas 
                    ref="canvasElement" 
                    :width="cameraWidth" 
                    :height="cameraHeight"
                    style="display: none;"
                  ></canvas>
                </div>

                <!-- 识别状态 -->
                <div class="recognition-status mb-4">
                  <v-progress-circular 
                    v-if="isRecognizing" 
                    indeterminate 
                    color="primary" 
                    size="64"
                  ></v-progress-circular>
                  
                  <v-icon 
                    v-else-if="recognitionResult === 'success'" 
                    size="64" 
                    color="green"
                  >
                    mdi-check-circle
                  </v-icon>
                  
                  <v-icon 
                    v-else-if="recognitionResult === 'failed'" 
                    size="64" 
                    color="red"
                  >
                    mdi-close-circle
                  </v-icon>
                  
                  <v-icon 
                    v-else 
                    size="64" 
                    color="grey"
                  >
                    mdi-face-recognition
                  </v-icon>
                </div>

                <!-- 状态文字 -->
                <div class="text-h6 mb-4">
                  {{ getStatusText() }}
                </div>

                <!-- 操作按钮 -->
                <v-btn 
                  v-if="!isRecognizing" 
                  color="primary" 
                  large 
                  @click="startFaceRecognition"
                  :disabled="!cameraReady"
                >
                  <v-icon left>mdi-camera</v-icon>
                  开始识别
                </v-btn>
              </v-card-text>
            </v-card>
          </v-col>
        </v-row>
      </v-container>

      <!-- 成功提示对话框 -->
      <v-dialog v-model="showSuccessDialog" max-width="400" persistent>
        <v-card class="text-center pa-4">
          <v-icon size="80" color="green" class="mb-4">mdi-check-circle</v-icon>
          <v-card-title class="text-h5 justify-center mb-2">
            签到成功！
          </v-card-title>
          <v-card-text class="text-h6">
            欢迎 {{ recognizedUser }}！<br>
            门已开启，请进入会议室。
          </v-card-text>
          <v-card-actions class="justify-center">
            <v-btn color="green" large @click="closeSuccessDialog">
              确定
            </v-btn>
          </v-card-actions>
        </v-card>
      </v-dialog>

      <!-- 失败提示 -->
      <v-snackbar v-model="showErrorSnackbar" color="error" timeout="3000">
        {{ errorMessage }}
        <template v-slot:actions>
          <v-btn color="white" variant="text" @click="showErrorSnackbar = false">
            关闭
          </v-btn>
        </template>
      </v-snackbar>
    </v-main>
  </v-app>
</template>

<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import { useRoute } from 'vue-router'

const route = useRoute()
const roomId = route.params.roomId

// 响应式数据
const currentTime = ref('')
const roomInfo = ref({
  id: roomId,
  name: '会议室A',
  location: 'A栋101',
  capacity: 10
})

const todayMeetings = ref([])
const videoElement = ref(null)
const canvasElement = ref(null)
const cameraReady = ref(false)
const isRecognizing = ref(false)
const recognitionResult = ref(null) // 'success', 'failed', null
const recognizedUser = ref('')
const showSuccessDialog = ref(false)
const showErrorSnackbar = ref(false)
const errorMessage = ref('')

// 摄像头尺寸
const cameraWidth = 320
const cameraHeight = 240

// 计算属性
const getStatusText = () => {
  if (isRecognizing.value) return '正在识别...'
  if (recognitionResult.value === 'success') return '识别成功！'
  if (recognitionResult.value === 'failed') return '识别失败，请重试'
  return '请点击开始识别按钮'
}

// 获取会议状态
const getMeetingStatus = (meeting) => {
  const now = new Date()
  const today = now.toISOString().split('T')[0]
  const currentTime = now.getHours() * 60 + now.getMinutes()
  
  if (meeting.date === today) {
    const [startTime, endTime] = meeting.timeSlots.split('-')
    const [startHour, startMin] = startTime.split(':').map(Number)
    const [endHour, endMin] = endTime.split(':').map(Number)
    const meetingStart = startHour * 60 + startMin
    const meetingEnd = endHour * 60 + endMin
    
    if (currentTime >= meetingStart && currentTime <= meetingEnd) {
      return 'current'
    } else if (currentTime < meetingStart) {
      return 'upcoming'
    }
  }
  return 'past'
}

// 更新时间
const updateTime = () => {
  const now = new Date()
  currentTime.value = now.toLocaleString('zh-CN', {
    year: 'numeric',
    month: '2-digit',
    day: '2-digit',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  })
}

// 加载今日会议
const loadTodayMeetings = () => {
  const bookings = JSON.parse(localStorage.getItem('myBookings') || '[]')
  const today = new Date().toISOString().split('T')[0]
  
  todayMeetings.value = bookings.filter(booking => 
    booking.roomId === roomId && 
    booking.date === today &&
    (booking.status === '已通过' || booking.status === 'approved')
  ).sort((a, b) => {
    const timeA = a.timeSlots.split('-')[0]
    const timeB = b.timeSlots.split('-')[0]
    return timeA.localeCompare(timeB)
  })
}

// 初始化摄像头
const initCamera = async () => {
  try {
    const stream = await navigator.mediaDevices.getUserMedia({ 
      video: { 
        width: cameraWidth, 
        height: cameraHeight,
        facingMode: 'user'
      } 
    })
    
    if (videoElement.value) {
      videoElement.value.srcObject = stream
      cameraReady.value = true
    }
  } catch (error) {
    console.error('摄像头初始化失败:', error)
    errorMessage.value = '摄像头初始化失败，请检查设备权限'
    showErrorSnackbar.value = true
  }
}

// 开始人脸识别
const startFaceRecognition = async () => {
  if (!cameraReady.value) return
  
  isRecognizing.value = true
  recognitionResult.value = null
  
  try {
    // 捕获当前帧
    const canvas = canvasElement.value
    const video = videoElement.value
    const ctx = canvas.getContext('2d')
    
    ctx.drawImage(video, 0, 0, cameraWidth, cameraHeight)
    const imageData = canvas.toDataURL('image/jpeg', 0.8)
    
    // 模拟调用后端人脸识别API
    const result = await mockFaceRecognition(imageData)
    
    if (result.success) {
      recognitionResult.value = 'success'
      recognizedUser.value = result.userName
      
      // 延迟显示成功对话框
      setTimeout(() => {
        showSuccessDialog.value = true
        // 模拟开门
        console.log('门已开启')
      }, 1000)
    } else {
      recognitionResult.value = 'failed'
      errorMessage.value = result.message || '人脸识别失败'
      showErrorSnackbar.value = true
    }
  } catch (error) {
    console.error('人脸识别错误:', error)
    recognitionResult.value = 'failed'
    errorMessage.value = '识别过程中发生错误'
    showErrorSnackbar.value = true
  } finally {
    isRecognizing.value = false
  }
}

// 模拟人脸识别API调用
const mockFaceRecognition = async (imageData) => {
  // 模拟API调用延迟
  await new Promise(resolve => setTimeout(resolve, 2000))
  
  // 模拟识别结果（实际应该调用后端API）
  const users = JSON.parse(localStorage.getItem('users_data') || '[]')
  const randomUser = users[Math.floor(Math.random() * users.length)]
  
  // 70%的成功率模拟
  if (Math.random() > 0.3 && randomUser) {
    return {
      success: true,
      userName: randomUser.name,
      userId: randomUser.id
    }
  } else {
    return {
      success: false,
      message: '未识别到注册用户或识别失败'
    }
  }
}

// 关闭成功对话框
const closeSuccessDialog = () => {
  showSuccessDialog.value = false
  recognitionResult.value = null
}

// 定时器
let timeInterval = null
let meetingInterval = null

onMounted(() => {
  // 更新时间
  updateTime()
  timeInterval = setInterval(updateTime, 1000)
  
  // 加载会议数据
  loadTodayMeetings()
  meetingInterval = setInterval(loadTodayMeetings, 30000) // 每30秒刷新一次
  
  // 初始化摄像头
  initCamera()
})

onUnmounted(() => {
  if (timeInterval) clearInterval(timeInterval)
  if (meetingInterval) clearInterval(meetingInterval)
  
  // 停止摄像头
  if (videoElement.value && videoElement.value.srcObject) {
    const tracks = videoElement.value.srcObject.getTracks()
    tracks.forEach(track => track.stop())
  }
})
</script>

<style scoped>
.room-terminal {
  background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
  min-height: 100vh;
}

.camera-container {
  border: 3px solid #1976d2;
  border-radius: 8px;
  overflow: hidden;
  background: #000;
}

.camera-preview {
  display: block;
  border-radius: 4px;
}

.recognition-status {
  min-height: 80px;
  display: flex;
  align-items: center;
  justify-content: center;
}

.v-timeline {
  max-height: 400px;
  overflow-y: auto;
}

/* 适配平板横屏 */
@media (orientation: landscape) and (max-width: 1024px) {
  .room-terminal {
    padding: 8px;
  }
}

/* 适配平板竖屏 */
@media (orientation: portrait) and (max-width: 768px) {
  .v-row {
    flex-direction: column;
  }
  
  .v-col {
    width: 100% !important;
    max-width: 100% !important;
  }
}
</style>