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