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

const bookings = ref([]);
const loading = ref(true);
const cancelDialog = ref(false);
const bookingToCancel = ref(null);

const fetchMyBookings = () => {
  loading.value = true;
  // 模拟从localStorage获取数据
  setTimeout(() => {
    const storedBookings = JSON.parse(localStorage.getItem('myBookings') || '[]');
    // 实际应用中，这里应该根据当前登录用户过滤
    // const currentUser = localStorage.getItem('username');
    // bookings.value = storedBookings.filter(b => b.user === currentUser);
    bookings.value = storedBookings.sort((a, b) => new Date(b.date) - new Date(a.date) || b.id - a.id); // 按日期和ID排序
    loading.value = false;
  }, 500);
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
  // 简单逻辑：只能取消“已预定”状态的，并且预定时间在未来
  if (booking.status !== '已预定') return false;
  // 假设预定时间至少是当天
  const bookingDateTime = new Date(`${booking.date}T${booking.timeSlots[0].split('-')[0].trim()}:00`);
  return bookingDateTime > new Date();
};

const confirmCancelBooking = (booking) => {
  bookingToCancel.value = booking;
  cancelDialog.value = true;
};

const executeCancelBooking = () => {
  if (!bookingToCancel.value) return;
  // 模拟API调用取消预定
  console.log('Cancelling booking:', bookingToCancel.value.id);

  // 更新localStorage中的数据
  const storedBookings = JSON.parse(localStorage.getItem('myBookings') || '[]');
  const updatedBookings = storedBookings.map(b => {
    if (b.id === bookingToCancel.value.id) {
      return { ...b, status: '已取消' };
    }
    return b;
  });
  localStorage.setItem('myBookings', JSON.stringify(updatedBookings));

  // 更新当前列表中的状态
  const index = bookings.value.findIndex(b => b.id === bookingToCancel.value.id);
  if (index !== -1) {
    bookings.value[index].status = '已取消';
  }

  cancelDialog.value = false;
  bookingToCancel.value = null;
  // 可以添加一个提示消息
};

onMounted(() => {
  fetchMyBookings();
});
</script>

<style scoped>
/* Add any specific styles here */
</style>