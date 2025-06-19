import { API_ENDPOINTS } from './config.js';

// 用户登录
export const loginUser = async (username, password) => {
  try {
    const response = await fetch(API_ENDPOINTS.LOGIN, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ username, password })
    });

    if (!response.ok) {
      const errorData = await response.json();
      throw new Error(errorData.error || '登录失败');
    }

    return await response.json();
  } catch (error) {
    console.error('登录请求失败:', error);
    throw error;
  }
};

// 获取用户列表
export const getUsers = async () => {
  try {
    const response = await fetch(API_ENDPOINTS.USERS);
    
    if (!response.ok) {
      throw new Error('获取用户列表失败');
    }

    const result = await response.json();
    
    // 返回完整的响应对象，包含success和data字段
    return result;
  } catch (error) {
    console.error('获取用户列表失败:', error);
    throw error;
  }
};

// 获取用户数据数组（兼容旧版本）
export const getUsersData = async () => {
  try {
    const result = await getUsers();
    
    // 检查响应格式并返回用户数组
    if (result.success && result.data) {
      return result.data;
    } else {
      throw new Error(result.message || '获取用户列表失败');
    }
  } catch (error) {
    console.error('获取用户列表失败:', error);
    throw error;
  }
};

// 创建用户
export const createUser = async (username, password) => {
  try {
    const response = await fetch(API_ENDPOINTS.USERS, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ username, password })
    });

    if (!response.ok) {
      const errorData = await response.json();
      throw new Error(errorData.message || errorData.error || '创建用户失败');
    }

    const result = await response.json();
    
    // 检查响应格式并返回用户数据
    if (result.success && result.data) {
      return result.data;
    } else {
      throw new Error(result.message || '创建用户失败');
    }
  } catch (error) {
    console.error('创建用户失败:', error);
    throw error;
  }
};

// 更新用户
export const updateUser = async (userId, username, password) => {
  try {
    const response = await fetch(API_ENDPOINTS.USER_BY_ID(userId), {
      method: 'PUT',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({ username, password })
    });

    if (!response.ok) {
      const errorData = await response.json();
      throw new Error(errorData.message || errorData.error || '更新用户失败');
    }

    const result = await response.json();
    
    // 检查响应格式
    if (result.success) {
      return result;
    } else {
      throw new Error(result.message || '更新用户失败');
    }
  } catch (error) {
    console.error('更新用户失败:', error);
    throw error;
  }
};

// 删除用户
export const deleteUser = async (userId) => {
  try {
    const response = await fetch(API_ENDPOINTS.USER_BY_ID(userId), {
      method: 'DELETE'
    });

    if (!response.ok) {
      const errorData = await response.json();
      throw new Error(errorData.message || errorData.error || '删除用户失败');
    }

    const result = await response.json();
    
    // 检查响应格式
    if (result.success) {
      return result;
    } else {
      throw new Error(result.message || '删除用户失败');
    }
  } catch (error) {
    console.error('删除用户失败:', error);
    throw error;
  }
};