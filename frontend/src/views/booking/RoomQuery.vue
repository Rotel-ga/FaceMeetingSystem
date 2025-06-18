<template>
  <v-container>
    <v-card>
      <v-card-title>会议室查询与预定</v-card-title>
      <v-card-text>
        <v-form ref="queryForm">
          <v-row align="center">
            <v-col cols="12" md="3">
              <v-text-field
                v-model="queryDate"
                label="选择日期"
                type="date"
                :rules="[rules.required]"
              ></v-text-field>
            </v-col>
            <v-col cols="12" md="3">
              <v-text-field
                v-model="queryStartTime"
                label="开始时间 (HH:MM)"
                type="time"
                :rules="[rules.required, rules.timeFormat]"
              ></v-text-field>
            </v-col>
            <v-col cols="12" md="3">
              <v-text-field
                v-model="queryEndTime"
                label="结束时间 (HH:MM)"
                type="time"
                :rules="[rules.required, rules.timeFormat, timeOrderRule]"
              ></v-text-field>
            </v-col>
            <v-col cols="12" md="2">
              <v-text-field
                v-model.number="queryCapacity"
                label="所需容量"
                type="number"
                min="0"
              ></v-text-field>
            </v-col>
            <v-col cols="12" md="1" class="text-right">
              <v-btn color="primary" @click="searchRooms" :loading="loadingSearch">查询</v-btn>
            </v-col>
          </v-row>
        </v-form>

        <v-divider class="my-4"></v-divider>

        <div v-if="loadingSearch" class="text-center">
          <v-progress-circular indeterminate color="primary"></v-progress-circular>
          <p>正在查询...</p>
        </div>

        <div v-if="!loadingSearch && searched && availableRooms.length > 0">
          <h3>可用会议室:</h3>
          <v-list lines="two">
            <v-list-item
              v-for="room in availableRooms"
              :key="room.id"
              @click="selectRoomForBooking(room)"
              :title="`${room.name} (容量: ${room.capacity})`"
              :subtitle="`位置: ${room.location || 'N/A'} - 设备: ${room.equipment ? room.equipment.join(', ') : '无'}`"
            >
              <template v-slot:append>
                <v-btn color="secondary" size="small">预定</v-btn>
              </template>
            </v-list-item>
          </v-list>
        </div>
        <div v-if="!loadingSearch && searched && availableRooms.length === 0">
          <v-alert type="info" prominent border="start">
            抱歉，在您选择的时间段内没有符合条件的可用会议室。
          </v-alert>
        </div>
      </v-card-text>
    </v-card>

    <!-- 预定确认对话框 -->
    <v-dialog v-model="showBookingDialog" max-width="500px" persistent>
        <v-card>
            <v-card-title class="text-h5">确认预定</v-card-title>
            <v-card-text v-if="selectedRoom">
                <p><strong>会议室:</strong> {{ selectedRoom.name }}</p>
                <p><strong>容量:</strong> {{ selectedRoom.capacity }}</p>
                <p><strong>日期:</strong> {{ queryDate ? new Date(queryDate + 'T00:00:00').toLocaleDateString() : '' }}</p>
                <p><strong>时间:</strong> {{ queryStartTime }} - {{ queryEndTime }}</p>
                <v-text-field v-model="bookingUserName" label="预定人姓名" :rules="[rules.required]"></v-text-field>
                <v-textarea v-model="bookingPurpose" label="预定事由 (可选)"></v-textarea>
            </v-card-text>
            <v-card-actions>
                <v-spacer></v-spacer>
                <v-btn color="grey darken-1" text @click="cancelBookingDialog">取消</v-btn>
                <v-btn color="blue darken-1" :loading="loadingBooking" text @click="confirmBooking">确认预定</v-btn>
            </v-card-actions>
        </v-card>
    </v-dialog>

    <v-snackbar v-model="snackbar.show" :color="snackbar.color" :timeout="snackbar.timeout">
      {{ snackbar.text }}
      <template v-slot:actions>
        <v-btn color="white" text @click="snackbar.show = false">关闭</v-btn>
      </template>
    </v-snackbar>
  </v-container>
</template>

<script setup>
import { ref, computed } from 'vue';

const queryForm = ref(null);
const queryDate = ref(new Date().toISOString().split('T')[0]); // 默认今天
const queryStartTime = ref('09:00');
const queryEndTime = ref('10:00');
const queryCapacity = ref(0);
const availableRooms = ref([]);
const searched = ref(false);
const loadingSearch = ref(false);
const loadingBooking = ref(false);

const showBookingDialog = ref(false);
const selectedRoom = ref(null);
const bookingUserName = ref('');
const bookingPurpose = ref('');

const snackbar = ref({
  show: false,
  text: '',
  color: 'success',
  timeout: 3000,
});

const rules = {
  required: value => !!value || '此项为必填项.',
  timeFormat: value => /^([01]\d|2[0-3]):([0-5]\d)$/.test(value) || '时间格式应为 HH:MM.',
};

const timeOrderRule = computed(() => {
  return value => {
    if (!queryStartTime.value || !value) return true;
    return queryStartTime.value < value || '结束时间必须晚于开始时间.';
  };
});

// 模拟的会议室数据，实际应从后端获取
const allRooms = [
  { id: 1, name: '会议室 Alpha', capacity: 10, location: 'A栋101', equipment: ['投影仪', '白板'], bookings: [] },
  { id: 2, name: '会议室 Beta', capacity: 20, location: 'B栋202', equipment: ['视频会议系统', '麦克风'], bookings: [] },
  { id: 3, name: '会议室 Gamma', capacity: 5, location: 'C栋303', equipment: ['白板'], bookings: [] },
  { id: 4, name: '会议室 Delta', capacity: 15, location: 'A栋102', equipment: ['投影仪', '视频会议系统'], bookings: [] },
];

async function searchRooms() {
  const { valid } = await queryForm.value.validate();
  if (!valid) {
    showSnackbar('请检查输入项是否正确。', 'error');
    return;
  }

  loadingSearch.value = true;
  searched.value = false;
  availableRooms.value = [];

  // 模拟API调用
  await new Promise(resolve => setTimeout(resolve, 1000));

  const searchStartDateTime = new Date(`${queryDate.value}T${queryStartTime.value}:00`);
  const searchEndDateTime = new Date(`${queryDate.value}T${queryEndTime.value}:00`);

  availableRooms.value = allRooms.filter(room => 
    room.capacity >= (queryCapacity.value || 0) &&
    !isRoomBooked(room, searchStartDateTime, searchEndDateTime)
  );
  
  loadingSearch.value = false;
  searched.value = true;
  if (availableRooms.value.length === 0 && searched.value) {
     showSnackbar('没有找到符合条件的会议室。', 'info');
  }
}

function isRoomBooked(room, start, end) {
  return room.bookings.some(b => {
    const existingStart = new Date(b.startTime);
    const existingEnd = new Date(b.endTime);
    // 检查时间段重叠: (StartA < EndB) and (EndA > StartB)
    return start < existingEnd && end > existingStart;
  });
}

function selectRoomForBooking(room) {
    selectedRoom.value = room;
    bookingUserName.value = ''; // 清空上次输入
    bookingPurpose.value = '';
    showBookingDialog.value = true;
}

function cancelBookingDialog() {
    showBookingDialog.value = false;
    selectedRoom.value = null;
}

async function confirmBooking() {
  if (!bookingUserName.value) {
    showSnackbar('请输入预定人姓名。', 'error');
    return;
  }
  loadingBooking.value = true;
  // 模拟API调用
  await new Promise(resolve => setTimeout(resolve, 1500));

  const bookingData = {
    roomId: selectedRoom.value.id,
    roomName: selectedRoom.value.name,
    userName: bookingUserName.value,
    purpose: bookingPurpose.value,
    startTime: `${queryDate.value}T${queryStartTime.value}:00`,
    endTime: `${queryDate.value}T${queryEndTime.value}:00`,
    status: '待审核', // 新预定默认为待审核
  };
  
  // 更新模拟数据 (实际应用中后端会处理)
  const roomToUpdate = allRooms.find(r => r.id === selectedRoom.value.id);
  if (roomToUpdate) {
    roomToUpdate.bookings.push({
        id: Date.now(), // 模拟预定ID
        userName: bookingData.userName,
        startTime: bookingData.startTime,
        endTime: bookingData.endTime,
    });
  }

  console.log('确认预定:', bookingData);
  showSnackbar(`会议室 ${selectedRoom.value.name} 预定请求已提交！`, 'success');
  
  loadingBooking.value = false;
  showBookingDialog.value = false;
  selectedRoom.value = null;
  // 预定成功后可以刷新查询结果或清空
  availableRooms.value = [];
  searched.value = false; 
}

function showSnackbar(text, color = 'success', timeout = 3000) {
  snackbar.value.text = text;
  snackbar.value.color = color;
  snackbar.value.timeout = timeout;
  snackbar.value.show = true;
}

</script>

<style scoped>
.v-col {
  padding-top: 8px;
  padding-bottom: 8px;
}
.text-right {
  display: flex;
  justify-content: flex-end;
  align-items: center; /* 垂直居中按钮 */
}
</style>