import { API_ENDPOINTS } from './config.js';

// 获取所有会议
export const getMeetings = async () => {
  try {
    const response = await fetch(API_ENDPOINTS.MEETINGS);
    const data = await response.json();
    
    if (data.success) {
      return data.data;
    } else {
      throw new Error(data.message || '获取会议列表失败');
    }
  } catch (error) {
    console.error('获取会议列表失败:', error);
    throw error;
  }
};

// 创建会议
export const createMeeting = async (roomId, topic, timeStart, timeEnd, userId) => {
  try {
    const response = await fetch(API_ENDPOINTS.MEETINGS, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        room_id: roomId,
        topic: topic,
        time_start: timeStart,
        time_end: timeEnd,
        user_id: userId
      }),
    });
    
    const data = await response.json();
    
    if (data.success) {
      return data.data;
    } else {
      throw new Error(data.message || '创建会议失败');
    }
  } catch (error) {
    console.error('创建会议失败:', error);
    throw error;
  }
};

// 更新会议
export const updateMeeting = async (meetingId, roomId, topic, timeStart, timeEnd, userId) => {
  try {
    const response = await fetch(API_ENDPOINTS.MEETING_BY_ID(meetingId), {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        room_id: roomId,
        topic: topic,
        time_start: timeStart,
        time_end: timeEnd,
        user_id: userId
      }),
    });
    
    const data = await response.json();
    
    if (data.success) {
      return data;
    } else {
      throw new Error(data.message || '更新会议失败');
    }
  } catch (error) {
    console.error('更新会议失败:', error);
    throw error;
  }
};

// 删除会议
export const deleteMeeting = async (meetingId) => {
  try {
    const response = await fetch(API_ENDPOINTS.MEETING_BY_ID(meetingId), {
      method: 'DELETE',
    });
    
    const data = await response.json();
    
    if (data.success) {
      return data;
    } else {
      throw new Error(data.message || '删除会议失败');
    }
  } catch (error) {
    console.error('删除会议失败:', error);
    throw error;
  }
};

// 根据会议室ID和日期查询会议
export const getMeetingsByRoom = async (roomId, date = null) => {
  try {
    const response = await fetch(API_ENDPOINTS.MEETINGS_BY_ROOM(roomId, date));
    const data = await response.json();
    
    if (data.success) {
      return data.data;
    } else {
      throw new Error(data.message || '获取会议室会议失败');
    }
  } catch (error) {
    console.error('获取会议室会议失败:', error);
    throw error;
  }
};

// 根据用户ID查询会议
export const getMeetingsByUser = async (userId) => {
  try {
    const response = await fetch(API_ENDPOINTS.MEETINGS_BY_USER(userId));
    const data = await response.json();
    
    if (data.success) {
      return data.data;
    } else {
      throw new Error(data.message || '获取用户会议失败');
    }
  } catch (error) {
    console.error('获取用户会议失败:', error);
    throw error;
  }
};

// 检查会议室在指定时间段是否可用
export const checkRoomAvailability = async (roomId, date, timeStart, timeEnd, excludeMeetingId = null) => {
  try {
    const meetings = await getMeetingsByRoom(roomId, date);
    
    // 检查时间冲突
    for (const meeting of meetings) {
      // 如果是更新会议，排除当前会议
      if (excludeMeetingId && meeting.id === excludeMeetingId) {
        continue;
      }
      
      const meetingStart = new Date(meeting.time_start);
      const meetingEnd = new Date(meeting.time_end);
      const newStart = new Date(timeStart);
      const newEnd = new Date(timeEnd);
      
      // 检查时间冲突
      if ((newStart < meetingEnd && newEnd > meetingStart)) {
        return {
          available: false,
          conflictMeeting: meeting
        };
      }
    }
    
    return {
      available: true,
      conflictMeeting: null
    };
  } catch (error) {
    console.error('检查会议室可用性失败:', error);
    throw error;
  }
};

// 获取会议室在指定日期的可用时间段
export const getAvailableTimeSlots = async (roomId, date) => {
  try {
    const meetings = await getMeetingsByRoom(roomId, date);
    
    // 定义工作时间段（9:00-18:00）
    const workStart = 9;
    const workEnd = 18;
    const timeSlots = [];
    
    // 生成所有可能的时间段（每小时一个时间段）
    for (let hour = workStart; hour < workEnd; hour++) {
      const startTime = `${hour.toString().padStart(2, '0')}:00`;
      const endTime = `${(hour + 1).toString().padStart(2, '0')}:00`;
      
      const slotStart = new Date(`${date} ${startTime}`);
      const slotEnd = new Date(`${date} ${endTime}`);
      
      // 检查这个时间段是否与现有会议冲突
      let isAvailable = true;
      for (const meeting of meetings) {
        const meetingStart = new Date(meeting.time_start);
        const meetingEnd = new Date(meeting.time_end);
        
        if ((slotStart < meetingEnd && slotEnd > meetingStart)) {
          isAvailable = false;
          break;
        }
      }
      
      timeSlots.push({
        start: startTime,
        end: endTime,
        available: isAvailable,
        label: `${startTime} - ${endTime}`
      });
    }
    
    return timeSlots;
  } catch (error) {
    console.error('获取可用时间段失败:', error);
    throw error;
  }
};