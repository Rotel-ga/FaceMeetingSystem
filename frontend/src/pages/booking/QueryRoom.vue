<template>
  <v-container>
    <v-card>
      <v-card-title>会议室查询</v-card-title>
      <v-card-text>
        <v-row>
          <v-col cols="12" md="6">
            <v-date-picker v-model="selectedDate" label="选择日期"></v-date-picker>
          </v-col>
          <v-col cols="12" md="6" class="d-flex align-center">
            <v-btn color="primary" @click="searchRooms">查询</v-btn>
          </v-col>
        </v-row>

        <v-divider class="my-4"></v-divider>

        <div v-if="loading" class="text-center">
          <v-progress-circular indeterminate color="primary"></v-progress-circular>
        </div>

        <v-alert v-if="!loading && availableRooms.length === 0 && searched" type="info">
          没有找到符合条件的会议室。
        </v-alert>

        <v-list v-if="!loading && availableRooms.length > 0">
          <v-list-item
            v-for="room in availableRooms"
            :key="room.id"
            @click="selectRoom(room)"
          >
            <v-list-item-content>
              <v-list-item-title>{{ room.name }}</v-list-item-title>
              <v-list-item-subtitle>状态: {{ room.isAvailable ? '可用' : '不可用' }}</v-list-item-subtitle>
            </v-list-item-content>
            <v-list-item-action>
              <v-btn v-if="room.isAvailable" color="success" small @click.stop="goToBooking(room)">预定</v-btn>
            </v-list-item-action>
          </v-list-item>
        </v-list>
      </v-card-text>
    </v-card>
  </v-container>
</template>

<script setup>
import { ref } from 'vue';
import { useRouter } from 'vue-router';

const selectedDate = ref(new Date().toISOString().substr(0, 10));
const availableRooms = ref([]);
const loading = ref(false);
const searched = ref(false);
const router = useRouter();

// 模拟会议室数据
const allRooms = [
  { id: '101', name: '会议室A', bookings: [{ date: '2024-07-20', timeSlots: ['09:00-10:00'] }] },
  { id: '102', name: '会议室B', bookings: [] },
  { id: '103', name: '会议室C', bookings: [{ date: '2024-07-20', timeSlots: ['14:00-16:00'] }] },
  { id: '201', name: '会议室D', bookings: [] },
];

const searchRooms = () => {
  loading.value = true;
  searched.value = true;
  // 模拟API调用延时
  setTimeout(() => {
    availableRooms.value = allRooms.filter(room => {
      // 简单演示，实际应用中需要更复杂的可用性检查逻辑
      const isBookedOnSelectedDate = room.bookings.some(b => b.date === selectedDate.value);
      return !isBookedOnSelectedDate; // 简化：如果当天有任何预定则认为不可用
    }).map(room => ({ ...room, isAvailable: true })); // 假设查询结果都是可用的，实际应根据时间段判断
    loading.value = false;
  }, 1000);
};

const selectRoom = (room) => {
  console.log('Selected room:', room);
  // 可以导航到房间详情或预定页面
};

const goToBooking = (room) => {
  router.push({ name: 'BookRoom', params: { roomId: room.id }, query: { date: selectedDate.value } });
};

// 初始化时执行一次查询，或根据需求调整
// searchRooms();
</script>

<style scoped>
.my-4 {
  margin-top: 1rem;
  margin-bottom: 1rem;
}
</style>