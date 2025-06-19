import { API_ENDPOINTS } from './config.js';

/**
 * 获取待审核的会议列表
 * @returns {Promise} 返回待审核会议列表
 */
export async function getPendingMeetings() {
  try {
    const response = await fetch(API_ENDPOINTS.ADMIN_PENDING_MEETINGS, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json',
      },
    });
    
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    
    const data = await response.json();
    return data;
  } catch (error) {
    console.error('获取待审核会议失败:', error);
    throw error;
  }
}

/**
 * 获取所有会议（管理员视图，可按状态筛选）
 * @param {string} status - 会议状态筛选条件（可选）
 * @returns {Promise} 返回会议列表
 */
export async function getAllMeetingsAdmin(status = '') {
  try {
    const url = status ? `${API_ENDPOINTS.ADMIN_MEETINGS}?status=${status}` : API_ENDPOINTS.ADMIN_MEETINGS;
    const response = await fetch(url, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json',
      },
    });
    
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    
    const data = await response.json();
    return data;
  } catch (error) {
    console.error('获取会议列表失败:', error);
    throw error;
  }
}

/**
 * 审核会议（批准或拒绝）
 * @param {number} meetingId - 会议ID
 * @param {string} status - 审核状态（'approved' 或 'rejected'）
 * @returns {Promise} 返回审核结果
 */
export async function reviewMeeting(meetingId, status) {
  try {
    if (!['approved', 'rejected'].includes(status)) {
      throw new Error('Invalid status. Must be "approved" or "rejected"');
    }
    
    const response = await fetch(API_ENDPOINTS.ADMIN_REVIEW_MEETING(meetingId), {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ status }),
    });
    
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    
    const data = await response.json();
    return data;
  } catch (error) {
    console.error('审核会议失败:', error);
    throw error;
  }
}

/**
 * 批准会议
 * @param {number} meetingId - 会议ID
 * @returns {Promise} 返回审核结果
 */
export async function approveMeeting(meetingId) {
  return reviewMeeting(meetingId, 'approved');
}

/**
 * 拒绝会议
 * @param {number} meetingId - 会议ID
 * @returns {Promise} 返回审核结果
 */
export async function rejectMeeting(meetingId) {
  return reviewMeeting(meetingId, 'rejected');
}