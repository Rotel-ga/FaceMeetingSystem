// 会议室终端配置文件
// 用于配置百度人脸识别API等参数

export const TERMINAL_CONFIG = {
  // 百度人脸识别API配置
  BAIDU_API: {
    // 访问令牌 - 已配置生产环境令牌
    ACCESS_TOKEN: process.env.VUE_APP_BAIDU_ACCESS_TOKEN || '24.1ad997f6ce51decc95501881db688263.2592000.1752998641.282335-119289764',
    
    // 人脸库组ID
    GROUP_ID: 'test',
    
    // 识别阈值配置
    THRESHOLDS: {
      // 最低识别分数
      MIN_SCORE: 60,
      // 活体检测等级 (NONE, LOW, NORMAL, HIGH)
      LIVENESS: 'NONE',
      // 质量控制等级 (NONE, LOW, NORMAL, HIGH)
      QUALITY: 'LOW'
    }
  },
  
  // 摄像头配置
  CAMERA: {
    WIDTH: 320,
    HEIGHT: 240,
    // 图像质量 (0.1 - 1.0)
    QUALITY: 0.8,
    // 摄像头方向 ('user' 前置, 'environment' 后置)
    FACING_MODE: 'user'
  },
  
  // 界面配置
  UI: {
    // 识别成功后对话框显示时间 (毫秒)
    SUCCESS_DIALOG_DURATION: 5000,
    // 错误提示显示时间 (毫秒)
    ERROR_SNACKBAR_DURATION: 3000,
    // 会议数据刷新间隔 (毫秒)
    MEETING_REFRESH_INTERVAL: 30000
  },
  
  // 开发模式配置
  DEVELOPMENT: {
    // 是否启用模拟模式
    ENABLE_MOCK: process.env.NODE_ENV === 'development',
    // 模拟识别成功率 (0-1)
    MOCK_SUCCESS_RATE: 0.7
  }
};

export default TERMINAL_CONFIG;