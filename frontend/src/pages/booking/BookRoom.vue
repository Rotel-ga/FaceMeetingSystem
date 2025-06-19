<template>
  <v-container>
    <v-card v-if="room">
      <v-card-title>预定会议室: {{ room.name }}</v-card-title>
      <v-card-subtitle>日期: {{ bookingDate }}</v-card-subtitle>
      <v-card-text>
        <v-form @submit.prevent="submitBooking">
          <p>请选择预定时间段：</p>
          <div v-if="loading" class="text-center">
            <v-progress-circular indeterminate color="primary"></v-progress-circular>
            <p>加载可用时间段...</p>
          </div>
          <v-chip-group v-else v-model="selectedTimeSlots" column multiple>
            <v-chip
              v-for="slot in availableSlots"
              :key="slot.value"
              :value="slot.value"
              filter
              outlined
              :disabled="slot.disabled"
            >
              {{ slot.text }}
            </v-chip>
          </v-chip-group>

          <v-textarea
            v-model="bookingReason"
            label="预定事由 (可选)"
            rows="2"
            class="mt-4"
          ></v-textarea>

          <v-alert v-if="bookingError" type="error" dense class="mt-3">
            {{ bookingError }}
          </v-alert>
          <v-alert v-if="bookingSuccess" type="success" dense class="mt-3">
            会议室预定成功！
          </v-alert>

          <v-btn type="submit" color="primary" class="mt-4" :loading="submitting" :disabled="selectedTimeSlots.length === 0">
            确认预定
          </v-btn>
          <v-btn text @click="router.back()" class="mt-4 ml-2">返回</v-btn>
        </v-form>
      </v-card-text>
    </v-card>
    <v-alert v-else type="warning">
      未找到会议室信息。
    </v-alert>
  </v-container>
</template>

<script setup>
import { ref, onMounted, computed } from 'vue';
import { useRoute, useRouter } from 'vue-router';
import { getRooms } from '@/api/roomService.js';
import { getMeetingsByRoom, createMeeting, getAvailableTimeSlots } from '@/api/meetingService.js';

const route = useRoute();
const router = useRouter();

const room = ref(null);
// 确保日期格式始终为 YYYY-MM-DD 字符串格式
const formatDateToString = (date) => {
  if (date instanceof Date) {
    return date.toISOString().substr(0, 10);
  }
  if (typeof date === 'string' && date.includes('T')) {
    return date.substr(0, 10);
  }
  return date || new Date().toISOString().substr(0, 10);
};

const bookingDate = ref(formatDateToString(route.query.date));
const selectedTimeSlots = ref([]);
const bookingReason = ref('');
const submitting = ref(false);
const bookingError = ref('');
const bookingSuccess = ref(false);
const availableSlots = ref([]);
const loading = ref(false);

const timeSlotsOptions = [
  { text: '09:00 - 10:00', value: '09:00-10:00', start: '09:00', end: '10:00' },
  { text: '10:00 - 11:00', value: '10:00-11:00', start: '10:00', end: '11:00' },
  { text: '11:00 - 12:00', value: '11:00-12:00', start: '11:00', end: '12:00' },
  { text: '13:00 - 14:00', value: '13:00-14:00', start: '13:00', end: '14:00' },
  { text: '14:00 - 15:00', value: '14:00-15:00', start: '14:00', end: '15:00' },
  { text: '15:00 - 16:00', value: '15:00-16:00', start: '15:00', end: '16:00' },
  { text: '16:00 - 17:00', value: '16:00-17:00', start: '16:00', end: '17:00' },
];

// 加载会议室信息和可用时间段
const loadRoomData = async () => {
  try {
    loading.value = true;
    const roomId = route.params.roomId;
    
    // 获取所有会议室
    const rooms = await getRooms();
    const foundRoom = rooms.find(r => r.id == roomId);
    
    if (foundRoom) {
      room.value = foundRoom;
      await loadAvailableTimeSlots();
    } else {
      console.error('Room not found');
      bookingError.value = '未找到指定的会议室';
    }
  } catch (error) {
    console.error('加载会议室数据失败:', error);
    bookingError.value = '加载会议室数据失败: ' + error.message;
  } finally {
    loading.value = false;
  }
};

// 加载可用时间段
const loadAvailableTimeSlots = async () => {
  try {
    const meetings = await getMeetingsByRoom(room.value.id, formatDateToString(bookingDate.value));
    
    // 检查每个时间段是否被占用
    availableSlots.value = timeSlotsOptions.map(slot => {
      const slotStart = new Date(`${formatDateToString(bookingDate.value)} ${slot.start}`);
      const slotEnd = new Date(`${formatDateToString(bookingDate.value)} ${slot.end}`);
      
      // 检查是否与现有会议冲突
      const isBooked = meetings.some(meeting => {
        const meetingStart = new Date(meeting.time_start);
        const meetingEnd = new Date(meeting.time_end);
        return (slotStart < meetingEnd && slotEnd > meetingStart);
      });
      
      return {
        ...slot,
        disabled: isBooked
      };
    });
  } catch (error) {
    console.error('加载可用时间段失败:', error);
    // 如果加载失败，显示所有时间段为可用
    availableSlots.value = timeSlotsOptions.map(slot => ({ ...slot, disabled: false }));
  }
};

onMounted(() => {
  loadRoomData();
});

const submitBooking = async () => {
  if (selectedTimeSlots.value.length === 0) {
    bookingError.value = '请至少选择一个时间段。';
    return;
  }
  
  submitting.value = true;
  bookingError.value = '';
  bookingSuccess.value = false;

  // 获取当前用户信息
  const userId = localStorage.getItem('userId');
  const userName = localStorage.getItem('userName') || 'unknown_user';
  
  if (!userId) {
    bookingError.value = '用户信息不完整，请重新登录';
    submitting.value = false;
    return;
  }

  try {
    // 为每个选中的时间段创建会议
    const createdMeetings = [];
    
    for (const timeSlot of selectedTimeSlots.value) {
      const slot = timeSlotsOptions.find(s => s.value === timeSlot);
      if (!slot) continue;
      
      const timeStart = `${formatDateToString(bookingDate.value)} ${slot.start}:00`;
      const timeEnd = `${formatDateToString(bookingDate.value)} ${slot.end}:00`;
      
      try {
        const meetingData = await createMeeting(
          room.value.id,
          bookingReason.value || `${userName}的会议`,
          timeStart,
          timeEnd,
          parseInt(userId)
        );
        
        createdMeetings.push({
          id: meetingData.id,
          timeSlot: timeSlot,
          timeStart: timeStart,
          timeEnd: timeEnd
        });
      } catch (error) {
        console.error(`创建时间段 ${timeSlot} 的会议失败:`, error);
        // 如果某个时间段创建失败，继续创建其他时间段
        bookingError.value += `时间段 ${slot.text} 预定失败: ${error.message}\n`;
      }
    }
    
    if (createdMeetings.length > 0) {
      bookingSuccess.value = true;
      selectedTimeSlots.value = []; // 清空选择
      bookingReason.value = '';
      
      // 重新加载可用时间段
      await loadAvailableTimeSlots();
      
      // 显示成功信息后跳转
      setTimeout(() => {
        router.push({ name: 'MyBookings' });
      }, 2000);
    } else {
      bookingError.value = bookingError.value || '所有时间段预定失败，请稍后再试。';
    }

  } catch (error) {
    console.error('预定失败:', error);
    bookingError.value = '预定失败: ' + error.message;
  } finally {
    submitting.value = false;
  }
};
</script>

<style scoped>
.mt-4 {
  margin-top: 1rem;
}
.ml-2 {
  margin-left: 0.5rem;
}
</style>