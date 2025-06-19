// API配置文件
const API_BASE_URL = 'http://localhost:18080';

export const API_ENDPOINTS = {
  // 用户相关API
  LOGIN: `${API_BASE_URL}/api/login`,
  USERS: `${API_BASE_URL}/api/users`,
  USER_BY_ID: (id) => `${API_BASE_URL}/api/users/${id}`,
  
  // 会议室相关API
  ROOMS: `${API_BASE_URL}/api/rooms`,
  ROOM_BY_ID: (id) => `${API_BASE_URL}/api/rooms/${id}`,
  
  // 会议相关API
  MEETINGS: `${API_BASE_URL}/api/meetings`,
  MEETING_BY_ID: (id) => `${API_BASE_URL}/api/meetings/${id}`,
  MEETINGS_BY_ROOM: (roomId, date) => `${API_BASE_URL}/api/meetings/room/${roomId}${date ? `?date=${date}` : ''}`,
  MEETINGS_BY_USER: (userId) => `${API_BASE_URL}/api/meetings/user/${userId}`,
  
  // 签到相关API
  CHECKINS: `${API_BASE_URL}/api/checkins`,
  CHECKIN_BY_ID: (id) => `${API_BASE_URL}/api/checkins/${id}`,
  
  // 管理员会议审核相关API
  ADMIN_MEETINGS: `${API_BASE_URL}/api/admin/meetings`,
  ADMIN_PENDING_MEETINGS: `${API_BASE_URL}/api/admin/meetings/pending`,
  ADMIN_REVIEW_MEETING: (id) => `${API_BASE_URL}/api/admin/meetings/${id}/review`
};

export default API_BASE_URL;