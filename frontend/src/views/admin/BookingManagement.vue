<template>
  <v-container>
    <v-card>
      <v-card-title>预定管理</v-card-title>
      <v-card-text>
        <v-data-table
          :headers="bookingHeaders"
          :items="bookings"
          class="elevation-1"
        >
          <template v-slot:item="{ item }">
            <tr>
              <td>{{ item.id }}</td>
              <td>{{ item.roomName }}</td>
              <td>{{ item.userName }}</td>
              <td>{{ item.reason || '无' }}</td>
              <td>{{ item.date }}</td>
              <td>{{ item.timeSlotDisplay }}</td>
              <td>
                <v-chip :color="getStatusColor(item.status)" dark>{{ item.status }}</v-chip>
              </td>
              <td>{{ item.createTimeDisplay }}</td>
              <td>
                <v-btn 
                  small 
                  color="success" 
                  @click="approveBooking(item)" 
                  :disabled="item.status !== '待审核'" 
                  class="mr-2"
                >
                  通过
                </v-btn>
                <v-btn 
                  small 
                  color="error" 
                  @click="rejectBooking(item)" 
                  :disabled="item.status !== '待审核'"
                >
                  拒绝
                </v-btn>
              </td>
            </tr>
          </template>
        </v-data-table>
      </v-card-text>
    </v-card>
  </v-container>
</template>

<script setup>
import { ref, onMounted, onActivated } from 'vue';

const bookingHeaders = ref([
  { title: '预定ID', value: 'id', key: 'id' },
  { title: '会议室', value: 'roomName', key: 'roomName' },
  { title: '预定人', value: 'userName', key: 'userName' },
  { title: '预定事由', value: 'reason', key: 'reason' },
  { title: '日期', value: 'date', key: 'date' },
  { title: '时间段', value: 'timeSlotDisplay', key: 'timeSlotDisplay' },
  { title: '状态', value: 'status', key: 'status' },
  { title: '创建时间', value: 'createTimeDisplay', key: 'createTimeDisplay' },
  { title: '操作', value: 'actions', sortable: false, key: 'actions' },
]);

const bookings = ref([]);

const loadBookings = () => {
  const storedBookings = localStorage.getItem('myBookings');
  if (storedBookings) {
    const parsedBookings = JSON.parse(storedBookings);
    bookings.value = parsedBookings.map(b => ({
      ...b,
      // 格式化显示
      userName: b.user || '未知用户',
      timeSlotDisplay: b.timeSlots ? b.timeSlots.join(', ') : 'N/A',
      createTimeDisplay: b.createTime ? new Date(b.createTime).toLocaleString() : 'N/A',
    }));
    // 按创建时间降序排序，最新的排在前面
    bookings.value.sort((a, b) => {
      return new Date(b.createTime || 0) - new Date(a.createTime || 0);
    });
  } else {
    bookings.value = [];
  }
};

const getStatusColor = (status) => {
  if (status === '待审核') return 'orange';
  if (status === '已通过') return 'green';
  if (status === '已拒绝') return 'red';
  if (status === '已取消') return 'grey';
  if (status === '已预定') return 'blue';
  return 'grey';
};

const approveBooking = (booking) => {
  console.log('审核通过:', booking);
  const item = bookings.value.find(b => b.id === booking.id);
  if (item) item.status = '已通过';
  
  // 更新 localStorage 中的状态
  const storedBookings = JSON.parse(localStorage.getItem('myBookings') || '[]');
  const bookingIndex = storedBookings.findIndex(sb => sb.id === booking.id);
  if (bookingIndex !== -1) {
    storedBookings[bookingIndex].status = '已通过';
    localStorage.setItem('myBookings', JSON.stringify(storedBookings));
  }
  loadBookings(); // 刷新列表
};

const rejectBooking = (booking) => {
  console.log('审核拒绝:', booking);
  const item = bookings.value.find(b => b.id === booking.id);
  if (item) item.status = '已拒绝';
  
  // 更新 localStorage 中的状态
  const storedBookings = JSON.parse(localStorage.getItem('myBookings') || '[]');
  const bookingIndex = storedBookings.findIndex(sb => sb.id === booking.id);
  if (bookingIndex !== -1) {
    storedBookings[bookingIndex].status = '已拒绝';
    localStorage.setItem('myBookings', JSON.stringify(storedBookings));
  }
  loadBookings(); // 刷新列表
};

onMounted(() => {
  loadBookings();
  // 添加事件监听器，以便在其他标签页中更新 localStorage 时，此页面也能刷新
  window.addEventListener('storage', (event) => {
    if (event.key === 'myBookings') {
      loadBookings();
    }
  });
});

// 如果使用了 <keep-alive>，在组件被激活时也重新加载数据
onActivated(() => {
  loadBookings();
});
</script>

<style scoped>
.mr-2 {
  margin-right: 8px;
}
</style>