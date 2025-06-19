<template>
  <v-container>
    <v-card>
      <v-card-title>会议室管理</v-card-title>
      <v-card-text>
        <v-data-table
          :headers="roomHeaders"
          :items="rooms"
          class="elevation-1"
        >
          <template v-slot:top>
            <v-toolbar flat>
              <v-toolbar-title>会议室列表</v-toolbar-title>
              <v-divider class="mx-4" inset vertical></v-divider>
              <v-spacer></v-spacer>
              <v-btn color="primary" dark class="mb-2" @click="showAddRoomDialog = true">
                添加会议室
              </v-btn>
            </v-toolbar>
          </template>
          <template v-slot:item.actions="{ item }">
            <v-icon small class="mr-2" @click="editRoom(item)">
              mdi-pencil
            </v-icon>
            <v-icon small @click="handleDeleteRoom(item)">
              mdi-delete
            </v-icon>
          </template>
        </v-data-table>

        <v-dialog v-model="showAddRoomDialog" max-width="600px">
          <v-card>
            <v-card-title>
              <span class="text-h5">{{ editingRoom ? '编辑会议室' : '添加会议室' }}</span>
            </v-card-title>
            <v-card-text>
              <v-container>
                <v-row>
                  <v-col cols="12">
                    <v-text-field v-model="currentRoom.name" label="会议室名称"></v-text-field>
                  </v-col>
                </v-row>
              </v-container>
            </v-card-text>
            <v-card-actions>
              <v-spacer></v-spacer>
              <v-btn color="blue darken-1" text @click="closeAddRoomDialog">取消</v-btn>
              <v-btn color="blue darken-1" text @click="saveRoom">保存</v-btn>
            </v-card-actions>
          </v-card>
        </v-dialog>
      </v-card-text>
    </v-card>
  </v-container>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue';
import { getRooms, createRoom, updateRoom, deleteRoom } from '@/api/roomService.js';

const roomHeaders = ref([
  { title: 'ID', value: 'id', key: 'id' },
  { title: '名称', value: 'name', key: 'name' },
  { title: '操作', value: 'actions', sortable: false, key: 'actions' },
]);

const rooms = ref([]);
const loading = ref(false);
const showAddRoomDialog = ref(false);
const editingRoom = ref(null);
const currentRoom = reactive({
  id: null,
  name: '',
});

// 加载会议室列表
const loadRooms = async () => {
  try {
    loading.value = true;
    const roomData = await getRooms();
    rooms.value = roomData;
  } catch (error) {
    console.error('获取会议室列表失败:', error);
    alert('获取会议室列表失败: ' + error.message);
  } finally {
    loading.value = false;
  }
};

// 页面加载时获取会议室列表
onMounted(() => {
  loadRooms();
});

const editRoom = (room) => {
  editingRoom.value = room;
  Object.assign(currentRoom, JSON.parse(JSON.stringify(room))); // 深拷贝以避免直接修改原对象
  showAddRoomDialog.value = true;
};

const handleDeleteRoom = async (room) => {
  if (confirm(`确定要删除会议室 "${room.name}" 吗？`)) {
    try {
      await deleteRoom(room.id);
      await loadRooms(); // 重新加载列表
      alert('会议室删除成功');
    } catch (error) {
      console.error('删除会议室失败:', error);
      alert('删除会议室失败: ' + error.message);
    }
  }
};

const closeAddRoomDialog = () => {
  showAddRoomDialog.value = false;
  editingRoom.value = null;
  // 重置currentRoom
  currentRoom.id = null;
  currentRoom.name = '';
};

const saveRoom = async () => {
  if (!currentRoom.name.trim()) {
    alert('请输入会议室名称');
    return;
  }

  try {
    if (editingRoom.value) {
      // 更新会议室
      await updateRoom(currentRoom.id, currentRoom.name);
      alert('会议室更新成功');
    } else {
      // 创建新会议室
      await createRoom(currentRoom.name);
      alert('会议室创建成功');
    }
    
    closeAddRoomDialog();
    await loadRooms(); // 重新加载列表
  } catch (error) {
    console.error('保存会议室失败:', error);
    alert('保存会议室失败: ' + error.message);
  }
};
</script>