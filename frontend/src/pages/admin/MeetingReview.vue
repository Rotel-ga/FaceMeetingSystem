<template>
  <div class="meeting-review">
    <div class="header">
      <h2>会议审核管理</h2>
      <div class="filter-tabs">
        <button 
          v-for="tab in tabs" 
          :key="tab.value"
          :class="['tab-btn', { active: activeTab === tab.value }]"
          @click="switchTab(tab.value)"
        >
          {{ tab.label }}
          <span v-if="tab.value === 'pending' && pendingCount > 0" class="badge">{{ pendingCount }}</span>
        </button>
      </div>
    </div>

    <div class="content">
      <div v-if="loading" class="loading">
        <div class="spinner"></div>
        <p>加载中...</p>
      </div>

      <div v-else-if="meetings.length === 0" class="empty-state">
        <div class="empty-icon">📋</div>
        <p>{{ getEmptyMessage() }}</p>
      </div>

      <div v-else class="meetings-grid">
        <div 
          v-for="meeting in meetings" 
          :key="meeting.id"
          class="meeting-card"
          :class="`status-${meeting.status}`"
        >
          <div class="meeting-header">
            <h3>{{ meeting.topic }}</h3>
            <span class="status-badge" :class="`status-${meeting.status}`">
              {{ getStatusText(meeting.status) }}
            </span>
          </div>
          
          <div class="meeting-details">
            <div class="detail-item">
              <span class="label">会议室:</span>
              <span class="value">{{ getRoomName(meeting.room_id) }}</span>
            </div>
            <div class="detail-item">
              <span class="label">申请人:</span>
              <span class="value">{{ getUserName(meeting.user_id) }}</span>
            </div>
            <div class="detail-item">
              <span class="label">开始时间:</span>
              <span class="value">{{ formatDateTime(meeting.time_start) }}</span>
            </div>
            <div class="detail-item">
              <span class="label">结束时间:</span>
              <span class="value">{{ formatDateTime(meeting.time_end) }}</span>
            </div>
          </div>

          <div v-if="meeting.status === 'pending'" class="actions">
            <button 
              class="btn btn-approve"
              @click="handleApprove(meeting.id)"
              :disabled="processing"
            >
              ✓ 批准
            </button>
            <button 
              class="btn btn-reject"
              @click="handleReject(meeting.id)"
              :disabled="processing"
            >
              ✗ 拒绝
            </button>
          </div>
        </div>
      </div>
    </div>

    <!-- 确认对话框 -->
    <div v-if="showConfirmDialog" class="modal-overlay" @click="closeConfirmDialog">
      <div class="confirm-dialog" @click.stop>
        <h3>确认操作</h3>
        <p>{{ confirmMessage }}</p>
        <div class="dialog-actions">
          <button class="btn btn-cancel" @click="closeConfirmDialog">取消</button>
          <button 
            class="btn"
            :class="confirmAction === 'approve' ? 'btn-approve' : 'btn-reject'"
            @click="confirmReview"
            :disabled="processing"
          >
            确认
          </button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { getPendingMeetings, getAllMeetingsAdmin, approveMeeting, rejectMeeting } from '@/api/adminService';
import { getRooms } from '@/api/roomService';
import { getUsers } from '@/api/userService';

export default {
  name: 'MeetingReview',
  data() {
    return {
      activeTab: 'pending',
      meetings: [],
      rooms: [],
      users: [],
      loading: false,
      processing: false,
      pendingCount: 0,
      showConfirmDialog: false,
      confirmAction: '',
      confirmMeetingId: null,
      confirmMessage: '',
      tabs: [
        { label: '待审核', value: 'pending' },
        { label: '已批准', value: 'approved' },
        { label: '已拒绝', value: 'rejected' },
        { label: '全部', value: 'all' }
      ]
    };
  },
  async mounted() {
    await this.loadBasicData();
    await this.loadMeetings();
  },
  methods: {
    async loadBasicData() {
      try {
        const [roomsData, usersResponse] = await Promise.all([
          getRooms(),
          getUsers()
        ]);
        
        // getRooms() 直接返回数据数组
        this.rooms = roomsData;
        
        if (usersResponse.success) {
          this.users = usersResponse.data;
        }
      } catch (error) {
        console.error('加载基础数据失败:', error);
      }
    },
    
    async loadMeetings() {
      this.loading = true;
      try {
        let response;
        if (this.activeTab === 'pending') {
          response = await getPendingMeetings();
        } else if (this.activeTab === 'all') {
          response = await getAllMeetingsAdmin();
        } else {
          response = await getAllMeetingsAdmin(this.activeTab);
        }
        
        if (response.success) {
          this.meetings = response.data;
          
          // 更新待审核数量
          if (this.activeTab !== 'pending') {
            const pendingResponse = await getPendingMeetings();
            if (pendingResponse.success) {
              this.pendingCount = pendingResponse.data.length;
            }
          } else {
            this.pendingCount = this.meetings.length;
          }
        }
      } catch (error) {
        console.error('加载会议列表失败:', error);
        this.$message?.error('加载会议列表失败');
      } finally {
        this.loading = false;
      }
    },
    
    switchTab(tab) {
      this.activeTab = tab;
      this.loadMeetings();
    },
    
    handleApprove(meetingId) {
      this.confirmAction = 'approve';
      this.confirmMeetingId = meetingId;
      this.confirmMessage = '确定要批准这个会议申请吗？';
      this.showConfirmDialog = true;
    },
    
    handleReject(meetingId) {
      this.confirmAction = 'reject';
      this.confirmMeetingId = meetingId;
      this.confirmMessage = '确定要拒绝这个会议申请吗？';
      this.showConfirmDialog = true;
    },
    
    async confirmReview() {
      this.processing = true;
      try {
        let response;
        if (this.confirmAction === 'approve') {
          response = await approveMeeting(this.confirmMeetingId);
        } else {
          response = await rejectMeeting(this.confirmMeetingId);
        }
        
        if (response.success) {
          this.$message?.success(
            this.confirmAction === 'approve' ? '会议已批准' : '会议已拒绝'
          );
          await this.loadMeetings();
        } else {
          this.$message?.error(response.message || '操作失败');
        }
      } catch (error) {
        console.error('审核操作失败:', error);
        this.$message?.error('操作失败，请重试');
      } finally {
        this.processing = false;
        this.closeConfirmDialog();
      }
    },
    
    closeConfirmDialog() {
      this.showConfirmDialog = false;
      this.confirmAction = '';
      this.confirmMeetingId = null;
      this.confirmMessage = '';
    },
    
    getRoomName(roomId) {
      const room = this.rooms.find(r => r.id === roomId);
      return room ? room.name : `会议室 ${roomId}`;
    },
    
    getUserName(userId) {
      const user = this.users.find(u => u.id === userId);
      return user ? user.username : `用户 ${userId}`;
    },
    
    getStatusText(status) {
      const statusMap = {
        pending: '待审核',
        approved: '已批准',
        rejected: '已拒绝'
      };
      return statusMap[status] || status;
    },
    
    getEmptyMessage() {
      const messages = {
        pending: '暂无待审核的会议',
        approved: '暂无已批准的会议',
        rejected: '暂无已拒绝的会议',
        all: '暂无会议记录'
      };
      return messages[this.activeTab] || '暂无数据';
    },
    
    formatDateTime(dateTimeStr) {
      try {
        const date = new Date(dateTimeStr);
        return date.toLocaleString('zh-CN', {
          year: 'numeric',
          month: '2-digit',
          day: '2-digit',
          hour: '2-digit',
          minute: '2-digit'
        });
      } catch (error) {
        return dateTimeStr;
      }
    }
  }
};
</script>

<style scoped>
.meeting-review {
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
  font-size: 24px;
}

.filter-tabs {
  display: flex;
  gap: 10px;
  border-bottom: 2px solid #f0f0f0;
}

.tab-btn {
  padding: 10px 20px;
  border: none;
  background: none;
  cursor: pointer;
  font-size: 14px;
  color: #666;
  border-bottom: 2px solid transparent;
  transition: all 0.3s ease;
  position: relative;
}

.tab-btn:hover {
  color: #409eff;
}

.tab-btn.active {
  color: #409eff;
  border-bottom-color: #409eff;
}

.badge {
  background: #f56c6c;
  color: white;
  border-radius: 10px;
  padding: 2px 6px;
  font-size: 12px;
  margin-left: 5px;
}

.loading {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 60px 20px;
  color: #666;
}

.spinner {
  width: 40px;
  height: 40px;
  border: 4px solid #f3f3f3;
  border-top: 4px solid #409eff;
  border-radius: 50%;
  animation: spin 1s linear infinite;
  margin-bottom: 15px;
}

@keyframes spin {
  0% { transform: rotate(0deg); }
  100% { transform: rotate(360deg); }
}

.empty-state {
  text-align: center;
  padding: 60px 20px;
  color: #999;
}

.empty-icon {
  font-size: 48px;
  margin-bottom: 15px;
}

.meetings-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(400px, 1fr));
  gap: 20px;
}

.meeting-card {
  border: 1px solid #e4e7ed;
  border-radius: 8px;
  padding: 20px;
  background: white;
  box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
  transition: all 0.3s ease;
}

.meeting-card:hover {
  box-shadow: 0 4px 8px rgba(0, 0, 0, 0.15);
}

.meeting-card.status-pending {
  border-left: 4px solid #e6a23c;
}

.meeting-card.status-approved {
  border-left: 4px solid #67c23a;
}

.meeting-card.status-rejected {
  border-left: 4px solid #f56c6c;
}

.meeting-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 15px;
}

.meeting-header h3 {
  margin: 0;
  color: #333;
  font-size: 16px;
}

.status-badge {
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 12px;
  font-weight: bold;
}

.status-badge.status-pending {
  background: #fdf6ec;
  color: #e6a23c;
}

.status-badge.status-approved {
  background: #f0f9ff;
  color: #67c23a;
}

.status-badge.status-rejected {
  background: #fef0f0;
  color: #f56c6c;
}

.meeting-details {
  margin-bottom: 15px;
}

.detail-item {
  display: flex;
  margin-bottom: 8px;
}

.detail-item .label {
  width: 80px;
  color: #666;
  font-size: 14px;
}

.detail-item .value {
  color: #333;
  font-size: 14px;
}

.actions {
  display: flex;
  gap: 10px;
  justify-content: flex-end;
}

.btn {
  padding: 8px 16px;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 14px;
  transition: all 0.3s ease;
}

.btn:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}

.btn-approve {
  background: #67c23a;
  color: white;
}

.btn-approve:hover:not(:disabled) {
  background: #5daf34;
}

.btn-reject {
  background: #f56c6c;
  color: white;
}

.btn-reject:hover:not(:disabled) {
  background: #f25c5c;
}

.btn-cancel {
  background: #dcdfe6;
  color: #606266;
}

.btn-cancel:hover {
  background: #c8c9cc;
}

.modal-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.confirm-dialog {
  background: white;
  border-radius: 8px;
  padding: 30px;
  max-width: 400px;
  width: 90%;
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.3);
}

.confirm-dialog h3 {
  margin: 0 0 15px 0;
  color: #333;
}

.confirm-dialog p {
  margin: 0 0 20px 0;
  color: #666;
  line-height: 1.5;
}

.dialog-actions {
  display: flex;
  gap: 10px;
  justify-content: flex-end;
}
</style>