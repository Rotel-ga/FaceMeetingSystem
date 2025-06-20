<template>
  <div class="checkin-management">
    <div class="header">
      <h2>签到记录管理</h2>
      <div class="search-section">
        <v-text-field
          v-model="searchKeyword"
          label="搜索会议主题或组织者"
          prepend-inner-icon="mdi-magnify"
          outlined
          dense
          clearable
          @input="filterMeetings"
        ></v-text-field>
        <v-select
          v-model="selectedDate"
          :items="dateOptions"
          item-title="text"
          item-value="value"
          label="选择日期"
          outlined
          dense
          @update:modelValue="loadMeetings"
        ></v-select>
      </div>
    </div>

    <div class="content">
      <div v-if="loading" class="loading">
        <v-progress-circular indeterminate color="primary"></v-progress-circular>
        <p>加载中...</p>
      </div>

      <div v-else-if="filteredMeetings.length === 0" class="empty-state">
        <div class="empty-icon">📋</div>
        <p>{{ searchKeyword ? '未找到匹配的会议' : '暂无会议记录' }}</p>
      </div>

      <div v-else class="meetings-list">
        <div 
          v-for="meeting in filteredMeetings" 
          :key="meeting.id"
          class="meeting-item"
        >
          <div class="meeting-info">
            <div class="meeting-header">
              <h3>{{ meeting.topic }}</h3>
              <div class="meeting-meta">
                <span class="time">{{ formatDateTime(meeting.time_start) }} - {{ formatTime(meeting.time_end) }}</span>
                <span class="room">{{ meeting.room_name }}</span>
                <span class="organizer">组织者: {{ meeting.organizer_name }}</span>
              </div>
            </div>
            
            <div class="checkin-summary">
              <div class="summary-item">
                <span class="label">签到人数:</span>
                <span class="value">{{ getCheckinCount(meeting.id) }}人</span>
              </div>
              <div class="summary-item">
                <span class="label">会议状态:</span>
                <span class="status" :class="getMeetingStatusClass(meeting)">{{ getMeetingStatusText(meeting) }}</span>
              </div>
            </div>
          </div>

          <div class="actions">
            <v-btn 
              color="primary" 
              outlined 
              small
              @click="viewCheckinDetails(meeting)"
            >
              查看详情
            </v-btn>
            <v-btn 
              color="success" 
              outlined 
              small
              @click="exportCheckinData(meeting)"
            >
              导出数据
            </v-btn>
          </div>
        </div>
      </div>
    </div>

    <!-- 签到详情对话框 -->
    <v-dialog v-model="showCheckinDialog" max-width="800px">
      <v-card>
        <v-card-title>
          <span class="text-h5">{{ selectedMeeting?.topic }} - 签到详情</span>
          <v-spacer></v-spacer>
          <v-btn icon @click="showCheckinDialog = false">
            <v-icon>mdi-close</v-icon>
          </v-btn>
        </v-card-title>
        
        <v-card-text>
          <div class="meeting-details">
            <div class="detail-row">
              <strong>会议时间:</strong> {{ formatDateTime(selectedMeeting?.time_start) }} - {{ formatTime(selectedMeeting?.time_end) }}
            </div>
            <div class="detail-row">
              <strong>会议室:</strong> {{ selectedMeeting?.room_name }}
            </div>
            <div class="detail-row">
              <strong>组织者:</strong> {{ selectedMeeting?.organizer_name }}
            </div>
            <div class="detail-row">
              <strong>签到统计:</strong> 共 {{ checkinDetails.length }} 人签到
            </div>
          </div>

          <v-divider class="my-4"></v-divider>

          <div class="checkin-list">
            <h4>签到记录</h4>
            <div v-if="checkinDetails.length === 0" class="no-checkins">
              <p>暂无签到记录</p>
            </div>
            <div v-else>
              <div 
                v-for="(checkin, index) in checkinDetails" 
                :key="checkin.id"
                class="checkin-record"
              >
                <div class="record-number">{{ index + 1 }}</div>
                <div class="record-info">
                  <div class="username">{{ checkin.username }}</div>
                  <div class="checkin-time">{{ formatDateTime(checkin.checkin_time) }}</div>
                </div>
                <div class="record-status">
                  <v-chip 
                    small 
                    :color="getCheckinStatusColor(checkin, selectedMeeting)"
                    text-color="white"
                  >
                    {{ getCheckinStatusText(checkin, selectedMeeting) }}
                  </v-chip>
                </div>
              </div>
            </div>
          </div>
        </v-card-text>

        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn color="primary" @click="exportCheckinData(selectedMeeting)">导出Excel</v-btn>
          <v-btn @click="showCheckinDialog = false">关闭</v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>

    <!-- 消息提示 -->
    <v-snackbar v-model="showMessage" :color="messageType" timeout="3000">
      {{ message }}
    </v-snackbar>
  </div>
</template>

<script>
import { API_ENDPOINTS } from '@/api/config'
import { reactive } from 'vue'

export default {
  name: 'CheckinManagement',
  data() {
    return {
      loading: false,
      meetings: [],
      filteredMeetings: [],
      checkinRecords: reactive({}),
      checkinDetails: [],
      selectedMeeting: null,
      showCheckinDialog: false,
      searchKeyword: '',
      selectedDate: 'today',
      message: '',
      messageType: 'success',
      showMessage: false,
      dateOptions: [
        { text: '今天', value: 'today' },
        { text: '昨天', value: 'yesterday' },
        { text: '本周', value: 'thisWeek' },
        { text: '本月', value: 'thisMonth' },
        { text: '全部', value: 'all' }
      ]
    }
  },
  mounted() {
    this.loadMeetings()
  },
  methods: {
    async loadMeetings() {
      this.loading = true
      try {
        // 加载会议列表
        const meetingsResponse = await fetch(API_ENDPOINTS.MEETINGS)
        const meetingsData = await meetingsResponse.json()
        
        if (meetingsData.success) {
          this.meetings = []
          
          // 获取会议详细信息
          for (const meeting of meetingsData.data) {
            if (this.shouldIncludeMeeting(meeting)) {
              // 获取会议室信息
              const roomResponse = await fetch(API_ENDPOINTS.ROOM_BY_ID(meeting.room_id))
              const roomData = await roomResponse.json()
              
              // 获取组织者信息
              const userResponse = await fetch(API_ENDPOINTS.USER_BY_ID(meeting.user_id))
              const userData = await userResponse.json()
              
              const meetingWithDetails = {
                ...meeting,
                room_name: roomData.success ? roomData.data.name : '未知会议室',
                organizer_name: userData.success ? userData.data.username : '未知用户'
              }
              
              this.meetings.push(meetingWithDetails)
              
              // 加载签到记录
              await this.loadCheckinRecords(meeting.id)
            }
          }
          
          this.filterMeetings()
        }
      } catch (error) {
        console.error('加载会议列表失败:', error)
        this.showMessageToUser('加载数据失败', 'error')
      } finally {
        this.loading = false
      }
    },

    shouldIncludeMeeting(meeting) {
      const meetingDate = new Date(meeting.time_start)
      const now = new Date()
      const today = new Date(now.getFullYear(), now.getMonth(), now.getDate())
      const yesterday = new Date(today.getTime() - 24 * 60 * 60 * 1000)
      const weekStart = new Date(today.getTime() - (today.getDay() * 24 * 60 * 60 * 1000))
      const monthStart = new Date(now.getFullYear(), now.getMonth(), 1)
      

      
      switch (this.selectedDate) {
        case 'today':
          const result = meetingDate >= today && meetingDate < new Date(today.getTime() + 24 * 60 * 60 * 1000)
          return result
        case 'yesterday':
          return meetingDate >= yesterday && meetingDate < today
        case 'thisWeek':
          return meetingDate >= weekStart
        case 'thisMonth':
          return meetingDate >= monthStart
        case 'all':
        default:
          return true
      }
    },

    async loadCheckinRecords(meetingId) {
      try {
        const response = await fetch(API_ENDPOINTS.CHECKINS_BY_MEETING(meetingId))
        const data = await response.json()
        if (data.success) {
          this.$set ? this.$set(this.checkinRecords, meetingId, data.data) : (this.checkinRecords[meetingId] = data.data)
          this.$forceUpdate()
        }
      } catch (error) {
        console.error(`加载会议${meetingId}的签到记录失败:`, error)
      }
    },

    filterMeetings() {
      if (!this.searchKeyword) {
        this.filteredMeetings = [...this.meetings]
      } else {
        const keyword = this.searchKeyword.toLowerCase()
        this.filteredMeetings = this.meetings.filter(meeting => 
          meeting.topic.toLowerCase().includes(keyword) ||
          meeting.organizer_name.toLowerCase().includes(keyword) ||
          meeting.room_name.toLowerCase().includes(keyword)
        )
      }
    },

    async viewCheckinDetails(meeting) {
      this.selectedMeeting = meeting
      this.checkinDetails = this.checkinRecords[meeting.id] || []
      this.showCheckinDialog = true
    },

    exportCheckinData(meeting) {
      const checkins = this.checkinRecords[meeting.id] || []
      
      if (checkins.length === 0) {
        this.showMessageToUser('该会议暂无签到记录', 'warning')
        return
      }
      
      // 创建CSV数据
      const headers = ['序号', '姓名', '签到时间', '签到状态']
      const csvData = [headers]
      
      checkins.forEach((checkin, index) => {
        csvData.push([
          index + 1,
          checkin.username,
          this.formatDateTime(checkin.checkin_time),
          this.getCheckinStatusText(checkin, meeting)
        ])
      })
      
      // 转换为CSV格式
      const csvContent = csvData.map(row => row.join(',')).join('\n')
      const blob = new Blob([`\uFEFF${csvContent}`], { type: 'text/csv;charset=utf-8;' })
      
      // 下载文件
      const link = document.createElement('a')
      const url = URL.createObjectURL(blob)
      link.setAttribute('href', url)
      link.setAttribute('download', `${meeting.topic}_签到记录_${this.formatDate(meeting.time_start)}.csv`)
      link.style.visibility = 'hidden'
      document.body.appendChild(link)
      link.click()
      document.body.removeChild(link)
      
      this.showMessageToUser('签到记录已导出', 'success')
    },

    getCheckinCount(meetingId) {
      return this.checkinRecords[meetingId]?.length || 0
    },

    getMeetingStatusClass(meeting) {
      const now = new Date()
      const startTime = new Date(meeting.time_start)
      const endTime = new Date(meeting.time_end)
      
      if (now < startTime) return 'upcoming'
      if (now >= startTime && now <= endTime) return 'ongoing'
      return 'ended'
    },

    getMeetingStatusText(meeting) {
      const now = new Date()
      const startTime = new Date(meeting.time_start)
      const endTime = new Date(meeting.time_end)
      
      if (now < startTime) return '即将开始'
      if (now >= startTime && now <= endTime) return '进行中'
      return '已结束'
    },

    getCheckinStatusColor(checkin, meeting) {
      const checkinTime = new Date(checkin.checkin_time)
      const startTime = new Date(meeting.time_start)
      const timeDiff = (checkinTime - startTime) / (1000 * 60) // 分钟差
      
      if (timeDiff <= 0) return 'success' // 准时或提前
      if (timeDiff <= 15) return 'warning' // 迟到15分钟内
      return 'error' // 迟到超过15分钟
    },

    getCheckinStatusText(checkin, meeting) {
      const checkinTime = new Date(checkin.checkin_time)
      const startTime = new Date(meeting.time_start)
      const timeDiff = (checkinTime - startTime) / (1000 * 60) // 分钟差
      
      if (timeDiff <= 0) return '准时'
      if (timeDiff <= 15) return `迟到${Math.ceil(timeDiff)}分钟`
      return `迟到${Math.ceil(timeDiff)}分钟`
    },

    formatDateTime(timeStr) {
      const date = new Date(timeStr)
      return date.toLocaleString('zh-CN', {
        year: 'numeric',
        month: '2-digit',
        day: '2-digit',
        hour: '2-digit',
        minute: '2-digit'
      })
    },

    formatTime(timeStr) {
      return timeStr.substring(11, 16) // 提取 HH:MM
    },

    formatDate(timeStr) {
      return timeStr.substring(0, 10) // 提取 YYYY-MM-DD
    },

    showMessageToUser(msg, type = 'success') {
      this.message = msg
      this.messageType = type
      this.showMessage = true
    }
  }
}
</script>

<style scoped>
.checkin-management {
  padding: 20px;
  max-width: 1200px;
  margin: 0 auto;
}

.header {
  margin-bottom: 30px;
}

.header h2 {
  margin: 0 0 20px 0;
  color: #333;
  font-size: 1.8rem;
  font-weight: 500;
}

.search-section {
  display: grid;
  grid-template-columns: 1fr auto;
  gap: 20px;
  align-items: end;
}

.loading {
  text-align: center;
  padding: 50px;
  color: #666;
}

.empty-state {
  text-align: center;
  padding: 50px;
  color: #666;
}

.empty-icon {
  font-size: 3rem;
  margin-bottom: 20px;
}

.meetings-list {
  display: flex;
  flex-direction: column;
  gap: 20px;
}

.meeting-item {
  background: white;
  border-radius: 12px;
  padding: 25px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.1);
  border: 1px solid #e0e0e0;
  transition: all 0.3s ease;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.meeting-item:hover {
  box-shadow: 0 4px 16px rgba(0,0,0,0.15);
  transform: translateY(-2px);
}

.meeting-info {
  flex: 1;
}

.meeting-header h3 {
  margin: 0 0 10px 0;
  color: #333;
  font-size: 1.3rem;
  font-weight: 500;
}

.meeting-meta {
  display: flex;
  gap: 20px;
  margin-bottom: 15px;
}

.meeting-meta span {
  color: #666;
  font-size: 0.9rem;
}

.time {
  color: #1976d2 !important;
  font-weight: 500;
}

.checkin-summary {
  display: flex;
  gap: 30px;
}

.summary-item {
  display: flex;
  align-items: center;
  gap: 8px;
}

.summary-item .label {
  color: #666;
  font-size: 0.9rem;
}

.summary-item .value {
  color: #333;
  font-weight: 500;
}

.status {
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 0.8rem;
  font-weight: 500;
}

.status.upcoming {
  background: #e3f2fd;
  color: #1976d2;
}

.status.ongoing {
  background: #e8f5e8;
  color: #388e3c;
}

.status.ended {
  background: #fafafa;
  color: #757575;
}

.actions {
  display: flex;
  gap: 10px;
}

.meeting-details {
  margin-bottom: 20px;
}

.detail-row {
  margin-bottom: 10px;
  font-size: 0.95rem;
}

.checkin-list h4 {
  margin: 0 0 15px 0;
  color: #333;
  font-size: 1.1rem;
}

.no-checkins {
  text-align: center;
  color: #666;
  padding: 20px;
  background: #f9f9f9;
  border-radius: 8px;
}

.checkin-record {
  display: flex;
  align-items: center;
  padding: 12px 0;
  border-bottom: 1px solid #f0f0f0;
}

.checkin-record:last-child {
  border-bottom: none;
}

.record-number {
  width: 40px;
  height: 40px;
  background: #f5f5f5;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 500;
  color: #666;
  margin-right: 15px;
}

.record-info {
  flex: 1;
}

.username {
  font-weight: 500;
  color: #333;
  margin-bottom: 4px;
}

.checkin-time {
  color: #666;
  font-size: 0.9rem;
}

.record-status {
  margin-left: 15px;
}

/* 响应式设计 */
@media (max-width: 768px) {
  .checkin-management {
    padding: 15px;
  }
  
  .search-section {
    grid-template-columns: 1fr;
  }
  
  .meeting-item {
    flex-direction: column;
    align-items: stretch;
    gap: 20px;
  }
  
  .meeting-meta {
    flex-direction: column;
    gap: 8px;
  }
  
  .checkin-summary {
    flex-direction: column;
    gap: 10px;
  }
  
  .actions {
    justify-content: center;
  }
}
</style>