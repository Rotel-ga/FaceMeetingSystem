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
            <v-btn color="primary" @click="searchRooms">查询并预定</v-btn>
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
              <v-list-item-subtitle>
                状态: {{ room.isAvailable ? '可用' : '不可用' }} | 
                可用时间段: {{ room.availableHours }}小时 | 
                已预定会议: {{ room.totalMeetings }}个
              </v-list-item-subtitle>
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
import { ref, onMounted } from 'vue';
import { useRouter } from 'vue-router';
import { getRooms } from '@/api/roomService.js';
import { getMeetingsByRoom } from '@/api/meetingService.js';

// 确保日期格式始终为 YYYY-MM-DD 字符串格式，避免时区问题
const formatDateToString = (date) => {
  let targetDate;
  
  if (date instanceof Date) {
    targetDate = date;
  } else if (typeof date === 'string') {
    if (date.includes('T')) {
      targetDate = new Date(date);
    } else {
      // 如果已经是 YYYY-MM-DD 格式，直接返回
      return date;
    }
  } else {
    targetDate = new Date();
  }
  
  // 使用本地时间而不是UTC时间，避免时区偏差
  const year = targetDate.getFullYear();
  const month = String(targetDate.getMonth() + 1).padStart(2, '0');
  const day = String(targetDate.getDate()).padStart(2, '0');
  
  return `${year}-${month}-${day}`;
};

// 初始化为今天的日期
const today = new Date();
const selectedDate = ref(formatDateToString(today));
const availableRooms = ref([]);
const loading = ref(false);
const searched = ref(false);
const router = useRouter();

// 搜索可用会议室
const searchRooms = async () => {
  try {
    loading.value = true;
    searched.value = true;
    
    // 获取所有会议室
    const allRooms = await getRooms();
    
    // 检查每个会议室在选定日期的可用性
    const roomsWithAvailability = await Promise.all(
      allRooms.map(async (room) => {
        try {
          // 获取该会议室在选定日期的会议
          const meetings = await getMeetingsByRoom(room.id, formatDateToString(selectedDate.value));
          
          // 计算可用时间段数量
          const workHours = 9; // 9小时工作时间 (9:00-18:00)
          const bookedHours = meetings.length; // 简化计算，假设每个会议占用1小时
          const availableHours = workHours - bookedHours;
          
          return {
            ...room,
            isAvailable: availableHours > 0,
            availableHours: availableHours,
            totalMeetings: meetings.length,
            meetings: meetings
          };
        } catch (error) {
          console.error(`获取会议室 ${room.id} 的会议失败:`, error);
          return {
            ...room,
            isAvailable: true, // 如果获取失败，假设可用
            availableHours: 9,
            totalMeetings: 0,
            meetings: []
          };
        }
      })
    );
    
    availableRooms.value = roomsWithAvailability;
  } catch (error) {
    console.error('搜索会议室失败:', error);
    alert('搜索会议室失败: ' + error.message);
  } finally {
    loading.value = false;
  }
};

const selectRoom = (room) => {
  console.log('Selected room:', room);
  // 可以显示会议室详情
};

const goToBooking = (room) => {
  router.push({ name: 'BookRoom', params: { roomId: room.id }, query: { date: formatDateToString(selectedDate.value) } });
};

// 页面加载时执行一次查询
onMounted(() => {
  searchRooms();
});
</script>

<style scoped>
.my-4 {
  margin-top: 1rem;
  margin-bottom: 1rem;
}
</style>