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
            <v-icon small @click="deleteRoom(item)">
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
import { ref, reactive } from 'vue';

const roomHeaders = ref([
  { title: 'ID', value: 'id', key: 'id' },
  { title: '名称', value: 'name', key: 'name' },
  { title: '操作', value: 'actions', sortable: false, key: 'actions' },
]);

const rooms = ref([
  // 示例数据
  { id: 1, name: '会议室A' },
  { id: 2, name: '会议室B' },
]);

const showAddRoomDialog = ref(false);
const editingRoom = ref(null);
const currentRoom = reactive({
  id: null,
  name: '',
});

const editRoom = (room) => {
  editingRoom.value = room;
  Object.assign(currentRoom, JSON.parse(JSON.stringify(room))); // 深拷贝以避免直接修改原对象
  showAddRoomDialog.value = true;
};

const deleteRoom = (room) => {
  console.log('删除会议室:', room);
  rooms.value = rooms.value.filter(r => r.id !== room.id);
  // 在实际应用中，您需要调用API删除后端数据
};

const closeAddRoomDialog = () => {
  showAddRoomDialog.value = false;
  editingRoom.value = null;
  // 重置currentRoom
  currentRoom.id = null;
  currentRoom.name = '';
};

const saveRoom = () => {
  if (editingRoom.value) {
    const index = rooms.value.findIndex(r => r.id === currentRoom.id);
    if (index !== -1) {
      rooms.value.splice(index, 1, { ...currentRoom });
    }
    console.log('编辑会议室:', currentRoom);
    // 在实际应用中，您需要调用API更新后端数据
  } else {
    const newRoom = { ...currentRoom, id: Date.now() }; // 简单生成ID
    rooms.value.push(newRoom);
    console.log('添加会议室:', newRoom);
    // 在实际应用中，您需要调用API创建后端数据
  }
  closeAddRoomDialog();
};
</script>