<template>
  <v-container>
    <v-card>
      <v-card-title>我的预定</v-card-title>
      <v-card-text>
        <div v-if="loading" class="text-center">
          <v-progress-circular indeterminate color="primary"></v-progress-circular>
        </div>
        <v-alert v-else-if="bookings.length === 0" type="info">
          您当前没有预定记录。
        </v-alert>
        <v-list v-else>
          <v-list-item v-for="booking in bookings" :key="booking.id">
            <v-list-item-content>
              <v-list-item-title>会议室: {{ booking.roomName }}</v-list-item-title>
              <v-list-item-subtitle>
                日期: {{ booking.date }} | 时间: {{ booking.timeSlots.join(', ') }}
              </v-list-item-subtitle>
              <div v-if="booking.reason">事由: {{ booking.reason }}</div>
              <div>状态: <v-chip small :color="getBookingStatusColor(booking.status)">{{ booking.status }}</v-chip></div>
            </v-list-item-content>
            <v-list-item-action v-if="canCancel(booking)">
              <v-btn small color="error" @click="confirmCancelBooking(booking)">取消预定</v-btn>
            </v-list-item-action>
          </v-list-item>
        </v-list>
      </v-card-text>
    </v-card>

    <v-dialog v-model="cancelDialog" max-width="400">
      <v-card>
        <v-card-title class="headline">确认取消预定</v-card-title>
        <v-card-text>
          您确定要取消对会议室 "{{ bookingToCancel?.roomName }}" 在 {{ bookingToCancel?.date }} {{ bookingToCancel?.timeSlots.join(', ') }} 的预定吗？
        </v-card-text>
        <v-card-actions>
          <v-spacer></v-spacer>
          <v-btn text @click="cancelDialog = false">关闭</v-btn>
          <v-btn color="error" text @click="executeCancelBooking">确认取消</v-btn>
        </v-card-actions>
      </v-card>
    </v-dialog>

  </v-container>
</template>

<script setup>
import { ref, onMounted } from 'vue';
import { getMeetingsByUser, deleteMeeting } from '@/api/meetingService';
import { getRooms } from '@/api/roomService';

const bookings = ref([]);
const loading = ref(true);
const cancelDialog = ref(false);
const bookingToCancel = ref(null);
const rooms = ref([]);

const fetchMyBookings = async () => {
  loading.value = true;
  try {
    const userId = localStorage.getItem('userId');
    if (!userId) {
      console.error('用户未登录');
      loading.value = false;
      return;
    }

    // 获取所有会议室信息用于显示名称
    const roomsData = await getRooms();
    rooms.value = roomsData;
    
    // 获取用户的会议
    const meetingsData = await getMeetingsByUser(parseInt(userId));
    
    // 转换会议数据为预定格式
    bookings.value = meetingsData.map(meeting => {
      const room = rooms.value.find(r => r.id === meeting.room_id);
      const startTime = new Date(meeting.time_start);
      const endTime = new Date(meeting.time_end);
      
      return {
        id: meeting.id,
        roomId: meeting.room_id,
        roomName: room ? room.name : `会议室${meeting.room_id}`,
        date: startTime.toISOString().split('T')[0],
        timeSlots: [`${startTime.getHours().toString().padStart(2, '0')}:${startTime.getMinutes().toString().padStart(2, '0')}-${endTime.getHours().toString().padStart(2, '0')}:${endTime.getMinutes().toString().padStart(2, '0')}`],
        reason: meeting.title,
        status: getStatusDisplayText(meeting.status), // 显示状态
        originalStatus: meeting.status, // 原始状态用于逻辑判断
        timeStart: meeting.time_start,
        timeEnd: meeting.time_end
      };
    }).sort((a, b) => new Date(b.timeStart) - new Date(a.timeStart));
    
  } catch (error) {
    console.error('获取预定信息失败:', error);
  } finally {
    loading.value = false;
  }
};

const getStatusDisplayText = (status) => {
  const statusMap = {
    'pending': '待审核',
    'approved': '已通过',
    'rejected': '已拒绝',
    'cancelled': '已取消'
  };
  return statusMap[status] || status;
};

const getBookingStatusColor = (status) => {
  if (status === '待审核') return 'orange';
  if (status === '已通过') return 'green';
  if (status === '已拒绝') return 'red';
  if (status === '已取消') return 'grey';
  if (status === '已预定') return 'primary';
  if (status === '已完成') return 'success';
  return 'default';
};

const canCancel = (booking) => {
  // 只能取消"approved"状态的，并且预定时间在未来
  if (booking.originalStatus !== 'approved') return false;
  // 检查预定时间是否在未来
  const bookingDateTime = new Date(booking.timeStart);
  return bookingDateTime > new Date();
};

const confirmCancelBooking = (booking) => {
  bookingToCancel.value = booking;
  cancelDialog.value = true;
};

const executeCancelBooking = async () => {
  if (!bookingToCancel.value) return;
  
  try {
    // 调用API删除会议
    await deleteMeeting(bookingToCancel.value.id);
    
    // 从列表中移除已取消的预定
    const index = bookings.value.findIndex(b => b.id === bookingToCancel.value.id);
    if (index !== -1) {
      bookings.value.splice(index, 1);
    }
    
    console.log('预定已成功取消');
  } catch (error) {
    console.error('取消预定失败:', error);
    // 可以添加错误提示
  } finally {
    cancelDialog.value = false;
    bookingToCancel.value = null;
  }
};

onMounted(() => {
  fetchMyBookings();
});
</script>

<style scoped>
/* Add any specific styles here */
</style>