<template>
  <v-container>
    <v-card v-if="room">
      <v-card-title>预定会议室: {{ room.name }}</v-card-title>
      <v-card-subtitle>日期: {{ bookingDate }}</v-card-subtitle>
      <v-card-text>
        <v-form @submit.prevent="submitBooking">
          <p>请选择预定时间段：</p>
          <v-chip-group v-model="selectedTimeSlots" column multiple>
            <v-chip
              v-for="slot in availableTimeSlots"
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

const route = useRoute();
const router = useRouter();

const room = ref(null);
const bookingDate = ref(route.query.date || new Date().toISOString().substr(0, 10));
const selectedTimeSlots = ref([]);
const bookingReason = ref('');
const submitting = ref(false);
const bookingError = ref('');
const bookingSuccess = ref(false);

// 模拟会议室数据和可用时间段
const allRoomsData = {
  '101': { id: '101', name: '会议室A', capacity: 10 },
  '102': { id: '102', name: '会议室B', capacity: 5 },
  '103': { id: '103', name: '会议室C', capacity: 20 },
  '201': { id: '201', name: '会议室D', capacity: 8 },
};

// 模拟已预定的时间段 (实际应从后端获取)
const bookedSlotsForRoomAndDate = ref({
  '101': { '2024-07-20': ['09:00-10:00'] },
  '103': { '2024-07-20': ['14:00-16:00'] },
});

const timeSlotsOptions = [
  { text: '09:00 - 10:00', value: '09:00-10:00' },
  { text: '10:00 - 11:00', value: '10:00-11:00' },
  { text: '11:00 - 12:00', value: '11:00-12:00' },
  { text: '13:00 - 14:00', value: '13:00-14:00' },
  { text: '14:00 - 15:00', value: '14:00-15:00' },
  { text: '15:00 - 16:00', value: '15:00-16:00' },
  { text: '16:00 - 17:00', value: '16:00-17:00' },
];

onMounted(() => {
  const roomId = route.params.roomId;
  if (allRoomsData[roomId]) {
    room.value = allRoomsData[roomId];
  } else {
    console.error('Room not found');
  }
});

const availableTimeSlots = computed(() => {
  if (!room.value) return [];
  const booked = bookedSlotsForRoomAndDate.value[room.value.id]?.[bookingDate.value] || [];
  return timeSlotsOptions.map(slot => ({
    ...slot,
    disabled: booked.includes(slot.value),
  }));
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
  const userName = localStorage.getItem('userName') || 'unknown_user';
  const userRole = localStorage.getItem('userRole') || 'user';
  
  // 从users_data中获取用户详细信息
  let userEmail = '';
  const storedUsers = JSON.parse(localStorage.getItem('users_data') || '[]');
  const currentUser = storedUsers.find(u => u.name === userName);
  if (currentUser) {
    userEmail = currentUser.email || '';
  }

  try {
    // 实际项目中，这里会调用API将预定信息发送到后端
    await new Promise(resolve => setTimeout(resolve, 1500)); // 模拟网络延迟

    // 假设预定成功，更新模拟的已预定数据
    if (!bookedSlotsForRoomAndDate.value[room.value.id]) {
      bookedSlotsForRoomAndDate.value[room.value.id] = {};
    }
    if (!bookedSlotsForRoomAndDate.value[room.value.id][bookingDate.value]) {
      bookedSlotsForRoomAndDate.value[room.value.id][bookingDate.value] = [];
    }
    selectedTimeSlots.value.forEach(slot => {
      if (!bookedSlotsForRoomAndDate.value[room.value.id][bookingDate.value].includes(slot)) {
        bookedSlotsForRoomAndDate.value[room.value.id][bookingDate.value].push(slot);
      }
    });
    
    // 将预定信息存储到localStorage，用于"我的预定"页面
    const myBookings = JSON.parse(localStorage.getItem('myBookings') || '[]');
    myBookings.push({
      id: Date.now().toString(), // 简单唯一ID
      roomId: room.value.id,
      roomName: room.value.name,
      date: bookingDate.value,
      timeSlots: [...selectedTimeSlots.value],
      reason: bookingReason.value,
      status: '待审核', // 修改为待审核状态
      user: userName, // 存储用户名
      userEmail: userEmail, // 存储用户邮箱
      userRole: userRole, // 存储用户角色
      createTime: new Date().toISOString() // 添加创建时间
    });
    localStorage.setItem('myBookings', JSON.stringify(myBookings));

    bookingSuccess.value = true;
    selectedTimeSlots.value = []; // 清空选择
    bookingReason.value = '';
    // 可以在成功后跳转或显示更详细的成功信息
    setTimeout(() => {
        router.push({ name: 'MyBookings' });
    }, 2000);

  } catch (error) {
    console.error('Booking failed:', error);
    bookingError.value = '预定失败，请稍后再试。';
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