import { API_ENDPOINTS } from './config.js';

// 获取所有会议室
export const getRooms = async () => {
  try {
    const response = await fetch(API_ENDPOINTS.ROOMS);
    
    if (!response.ok) {
      throw new Error('获取会议室列表失败');
    }

    const result = await response.json();
    
    // 检查响应格式并返回会议室数组
    if (result.success && result.data) {
      return result.data;
    } else {
      throw new Error(result.message || '获取会议室列表失败');
    }
  } catch (error) {
    console.error('获取会议室列表失败:', error);
    throw error;
  }
};

// 根据ID获取单个会议室
export const getRoomById = async (roomId) => {
  try {
    // 由于后端没有单个房间的GET端点，使用getRooms获取所有房间然后过滤
    const allRooms = await getRooms();
    const room = allRooms.find(r => r.id === parseInt(roomId));
    
    if (room) {
      return {
        id: room.id,
        name: room.name,
        location: '未知位置', // 数据库中只有id和name
        capacity: 10 // 默认容量
      };
    } else {
      throw new Error('未找到指定的会议室');
    }
  } catch (error) {
    console.error('获取会议室信息失败:', error);
    // 如果获取失败，返回默认数据
    return {
      id: parseInt(roomId),
      name: `会议室${roomId}`,
      location: '未知位置',
      capacity: 10
    };
  }
};

// 创建会议室
export const createRoom = async (name) => {
  try {
    const response = await fetch(API_ENDPOINTS.ROOMS, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ name })
    });

    if (!response.ok) {
      const errorData = await response.json();
      throw new Error(errorData.message || errorData.error || '创建会议室失败');
    }

    const result = await response.json();
    
    // 检查响应格式并返回会议室数据
    if (result.success && result.data) {
      return result.data;
    } else {
      throw new Error(result.message || '创建会议室失败');
    }
  } catch (error) {
    console.error('创建会议室失败:', error);
    throw error;
  }
};

// 更新会议室
export const updateRoom = async (roomId, name) => {
  try {
    const response = await fetch(API_ENDPOINTS.ROOM_BY_ID(roomId), {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ name })
    });

    if (!response.ok) {
      const errorData = await response.json();
      throw new Error(errorData.message || errorData.error || '更新会议室失败');
    }

    const result = await response.json();
    
    // 检查响应格式
    if (result.success) {
      return result;
    } else {
      throw new Error(result.message || '更新会议室失败');
    }
  } catch (error) {
    console.error('更新会议室失败:', error);
    throw error;
  }
};

// 删除会议室
export const deleteRoom = async (roomId) => {
  try {
    const response = await fetch(API_ENDPOINTS.ROOM_BY_ID(roomId), {
      method: 'DELETE'
    });

    if (!response.ok) {
      const errorData = await response.json();
      throw new Error(errorData.message || errorData.error || '删除会议室失败');
    }

    const result = await response.json();
    
    // 检查响应格式
    if (result.success) {
      return result;
    } else {
      throw new Error(result.message || '删除会议室失败');
    }
  } catch (error) {
    console.error('删除会议室失败:', error);
    throw error;
  }
};