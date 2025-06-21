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
                      <span class="text-h6">{{ meeting.timeSlots || (meeting.time_start + '-' + meeting.time_end) }}</span>
                    </template>
                    <v-card :color="getMeetingStatus(meeting) === 'current' ? 'green lighten-4' : ''">
                      <v-card-title class="text-h6">{{ meeting.reason || meeting.topic }}</v-card-title>
                      <v-card-subtitle>预定人: {{ meeting.user_name || meeting.user || '未知用户' }}</v-card-subtitle>
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
      <v-dialog v-model="showSuccessDialog" max-width="500" persistent>
        <v-card class="text-center pa-4">
          <v-icon size="80" color="green" class="mb-4">mdi-check-circle</v-icon>
          <v-card-title class="text-h5 justify-center mb-2">
            签到成功！
          </v-card-title>
          <v-card-text>
            <div class="text-h6 mb-3">
              欢迎 {{ recognizedUser }}！
            </div>
            <v-divider class="mb-3"></v-divider>
            <div v-if="checkinDetails" class="text-left">
              <v-row dense>
                <v-col cols="6">
                  <strong>会议主题：</strong>
                </v-col>
                <v-col cols="6">
                  {{ checkinDetails.meetingTopic || '未知会议' }}
                </v-col>
                <v-col cols="6">
                  <strong>会议室：</strong>
                </v-col>
                <v-col cols="6">
                  {{ checkinDetails.roomName || roomInfo.name }}
                </v-col>
                <v-col cols="6">
                  <strong>签到时间：</strong>
                </v-col>
                <v-col cols="6">
                  {{ checkinDetails.checkinTime || new Date().toLocaleString() }}
                </v-col>
                <v-col cols="6" v-if="checkinDetails.recognitionScore">
                  <strong>识别分数：</strong>
                </v-col>
                <v-col cols="6" v-if="checkinDetails.recognitionScore">
                  {{ checkinDetails.recognitionScore }}%
                </v-col>
              </v-row>
            </div>
            <v-divider class="my-3"></v-divider>
            <div class="text-h6 green--text">
              门已开启，请进入会议室。
            </div>
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
import { ref, onMounted, onUnmounted, onBeforeUnmount, computed } from 'vue'
import { useRoute, onBeforeRouteLeave } from 'vue-router'
import { getRoomById } from '@/api/roomService.js'
import { getMeetingsByRoom } from '@/api/meetingService.js'
import { getUserById } from '@/api/userService.js'
import { API_ENDPOINTS } from '@/api/config.js'
import { TERMINAL_CONFIG } from '@/config/terminal.js'

const route = useRoute()
const roomId = parseInt(route.params.roomId)

// 响应式数据
const currentTime = ref('')
const roomInfo = ref({
  id: roomId,
  name: '加载中...',
  location: '加载中...',
  capacity: 0
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
const checkinDetails = ref(null) // 签到详细信息

// 摄像头尺寸
const cameraWidth = TERMINAL_CONFIG.CAMERA.WIDTH
const cameraHeight = TERMINAL_CONFIG.CAMERA.HEIGHT

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
  
  // 兼容不同的日期字段格式
  const meetingDate = meeting.date || today
  
  if (meetingDate === today) {
    let startTime, endTime
    
    // 兼容不同的时间字段格式
    if (meeting.timeSlots) {
      [startTime, endTime] = meeting.timeSlots.split('-')
    } else if (meeting.time_start && meeting.time_end) {
      startTime = meeting.time_start
      endTime = meeting.time_end
    } else {
      return 'past'
    }
    
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
const loadTodayMeetings = async () => {
  try {
    const today = new Date().toISOString().split('T')[0]
    
    const meetings = await getMeetingsByRoom(roomId, today)
    
    // 过滤已通过的会议
    const filteredMeetings = meetings
      .filter(meeting => meeting.status === '已通过' || meeting.status === 'approved')
    
    // 为每个会议获取用户名
    const meetingsWithUserNames = await Promise.all(
      filteredMeetings.map(async (meeting) => {
        try {
          const userData = await getUserById(meeting.user_id)
          return {
            ...meeting,
            user_name: userData.username || '未知用户'
          }
        } catch (error) {
          return {
            ...meeting,
            user_name: '未知用户'
          }
        }
      })
    )
    
    // 按时间排序
    todayMeetings.value = meetingsWithUserNames.sort((a, b) => {
      const timeA = a.time_start || a.timeSlots?.split('-')[0] || ''
      const timeB = b.time_start || b.timeSlots?.split('-')[0] || ''
      return timeA.localeCompare(timeB)
    })
  } catch (error) {
    console.error('加载今日会议失败:', error)
    // 如果API调用失败，尝试从localStorage获取数据作为备用
    const bookings = JSON.parse(localStorage.getItem('myBookings') || '[]')
    const today = new Date().toISOString().split('T')[0]
    
    todayMeetings.value = bookings.filter(booking => 
      parseInt(booking.roomId) === roomId && 
      booking.date === today &&
      (booking.status === '已通过' || booking.status === 'approved')
    ).sort((a, b) => {
      // 兼容localStorage中的timeSlots字段
      const timeA = a.time_start || a.timeSlots?.split('-')[0] || ''
      const timeB = b.time_start || b.timeSlots?.split('-')[0] || ''
      return timeA.localeCompare(timeB)
    })
  }
}

// 初始化摄像头
const initCamera = async () => {
  try {
    const stream = await navigator.mediaDevices.getUserMedia({ 
      video: { 
        width: cameraWidth, 
        height: cameraHeight,
        facingMode: TERMINAL_CONFIG.CAMERA.FACING_MODE
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
    const imageData = canvas.toDataURL('image/jpeg', TERMINAL_CONFIG.CAMERA.QUALITY)
    
    // 模拟调用后端人脸识别API
    const result = await mockFaceRecognition(imageData)
    
    if (result.success) {
      recognitionResult.value = 'success'
      recognizedUser.value = result.userName
      checkinDetails.value = result
      
      // 延迟显示成功对话框
      setTimeout(() => {
        showSuccessDialog.value = true
        // 模拟开门
        console.log('门已开启')
        
        // 自动关闭成功对话框
        setTimeout(() => {
          if (showSuccessDialog.value) {
            closeSuccessDialog()
          }
        }, TERMINAL_CONFIG.UI.SUCCESS_DIALOG_DURATION)
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

// 模拟识别结果函数（开发模式使用）
const mockRecognitionResult = async () => {
  // 模拟API调用延迟
  await new Promise(resolve => setTimeout(resolve, 2000))
  
  // 模拟识别结果
  const users = JSON.parse(localStorage.getItem('users_data') || '[]')
  const randomUser = users[Math.floor(Math.random() * users.length)]
  
  // 使用配置的成功率
  if (Math.random() < TERMINAL_CONFIG.DEVELOPMENT.MOCK_SUCCESS_RATE && randomUser) {
    return {
      success: true,
      userName: randomUser.name || randomUser.username || '测试用户',
      userId: randomUser.id,
      meetingTopic: '测试会议',
      roomName: roomInfo.value.name,
      checkinTime: new Date().toLocaleString(),
      recognitionScore: Math.floor(Math.random() * 20) + 80 // 80-100分
    }
  } else {
    return {
      success: false,
      message: '未识别到注册用户或识别失败'
    }
  }
}

// 真实人脸识别API调用
const mockFaceRecognition = async (imageData) => {
  try {
    // 使用配置文件中的访问令牌
     const token = TERMINAL_CONFIG.BAIDU_API.ACCESS_TOKEN
    
    if (!token) {
      console.warn('请配置百度API访问令牌')
      // 如果是开发模式且启用了模拟，返回模拟数据
      if (TERMINAL_CONFIG.DEVELOPMENT.ENABLE_MOCK) {
        return await mockRecognitionResult()
      }
      return {
        success: false,
        message: '请配置百度API访问令牌'
      }
    }
    
    // 提取base64数据（去除data:image/jpeg;base64,前缀）
    const base64Data = imageData.split(',')[1]
    
    const response = await fetch(API_ENDPOINTS.FACE_CHECKIN, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({
        image: base64Data,
        access_token: token
      })
    })
    
    const result = await response.json()
    
    // 添加详细的错误日志
    if (!response.ok) {
      console.error('API请求失败:', {
        status: response.status,
        statusText: response.statusText,
        result: result
      })
    }
    
    if (result.success) {
      return {
        success: true,
        userName: result.data.username,
        userId: result.data.user_id,
        meetingTopic: result.data.meeting_topic,
        roomName: result.data.room_name,
        checkinTime: result.data.checkin_time,
        recognitionScore: result.data.recognition_score
      }
    } else {
      // 返回更详细的错误信息
      let errorMessage = result.message || '人脸识别失败'
      if (result.error_code) {
        errorMessage += ` (错误代码: ${result.error_code})`
      }
      if (result.error_msg) {
        errorMessage += ` - ${result.error_msg}`
      }
      if (result.baidu_error) {
        errorMessage += ` - 百度API错误: ${result.baidu_error}`
      }
      
      return {
        success: false,
        message: errorMessage
      }
    }
  } catch (error) {
    console.error('人脸识别API调用失败:', error)
    return {
      success: false,
      message: '网络请求失败，请检查服务器连接'
    }
  }
}

// 关闭成功对话框
const closeSuccessDialog = () => {
  showSuccessDialog.value = false
  recognitionResult.value = null
  checkinDetails.value = null
}

// 定时器
let timeInterval = null
let meetingInterval = null

// 摄像头清理函数
const cleanupCamera = () => {
  if (videoElement.value && videoElement.value.srcObject) {
    const tracks = videoElement.value.srcObject.getTracks()
    tracks.forEach(track => {
      track.stop()
      console.log('摄像头轨道已停止:', track.kind)
    })
    videoElement.value.srcObject = null
    cameraReady.value = false
    console.log('摄像头已完全关闭')
  }
}

// 页面可见性变化处理
const handleVisibilityChange = () => {
  if (document.hidden) {
    // 页面隐藏时暂停摄像头
    cleanupCamera()
  } else {
    // 页面显示时重新初始化摄像头
    setTimeout(() => {
      initCamera()
    }, 500)
  }
}

// 路由离开前的清理
onBeforeRouteLeave((to, from, next) => {
  cleanupCamera()
  next()
})

// 加载会议室信息
const loadRoomInfo = async () => {
  try {
    const roomData = await getRoomById(roomId)
    
    roomInfo.value = {
      id: roomId,
      name: roomData.name || `会议室${roomId}`,
      location: roomData.location || '未知位置',
      capacity: roomData.capacity || 0
    }
  } catch (error) {
    console.error('加载会议室信息失败:', error)
    // 使用默认信息
    roomInfo.value = {
      id: roomId,
      name: `会议室${roomId}`,
      location: '未知位置',
      capacity: 10
    }
  }
};

onMounted(async () => {
  // 加载会议室信息
  await loadRoomInfo()
  
  // 更新时间
  updateTime()
  timeInterval = setInterval(updateTime, 1000)
  
  // 加载会议数据
  await loadTodayMeetings()
  meetingInterval = setInterval(loadTodayMeetings, TERMINAL_CONFIG.UI.MEETING_REFRESH_INTERVAL)
  
  // 初始化摄像头
  initCamera()
  
  // 监听页面可见性变化
  document.addEventListener('visibilitychange', handleVisibilityChange)
  
  // 监听页面卸载事件
  window.addEventListener('beforeunload', cleanupCamera)
  window.addEventListener('pagehide', cleanupCamera)
})

onBeforeUnmount(() => {
  // 清理摄像头
  cleanupCamera()
})

onUnmounted(() => {
  if (timeInterval) clearInterval(timeInterval)
  if (meetingInterval) clearInterval(meetingInterval)
  
  // 移除事件监听器
  document.removeEventListener('visibilitychange', handleVisibilityChange)
  window.removeEventListener('beforeunload', cleanupCamera)
  window.removeEventListener('pagehide', cleanupCamera)
  
  // 确保摄像头完全关闭
  cleanupCamera()
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