#pragma once
#include <pgmspace.h>

// Raw HTML stored in PROGMEM to avoid Arduino .ino preprocessor interference
// Placeholders: %ESP32_IP%
const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Robot Control</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&family=Orbitron&display=swap" rel="stylesheet">
    <link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css" rel="stylesheet">
    <script src="/assets/three.min.js"></script>
    <script src="/assets/OrbitControls.js"></script>
    <style>
        :root {
            --bg-primary: linear-gradient(135deg, #0a0a0a 0%, #1a1a2e 100%);
            --bg-secondary: rgba(0, 0, 0, 0.8);
            --bg-card: rgba(255, 255, 255, 0.05);
            --bg-card-hover: rgba(0, 255, 255, 0.1);
            --border-primary: rgba(255, 255, 255, 0.1);
            --border-accent: rgba(0, 255, 255, 0.2);
            --text-primary: #ffffff;
            --text-secondary: #cccccc;
            --text-muted: #999999;
            --accent-primary: #00ffff;
            --accent-secondary: #ff6b35;
            --shadow-glow: rgba(0, 255, 255, 0.1);
            --success-color: #00ff88;
            --error-color: #ff4444;
        }
        
        body.light-mode {
            --bg-primary: linear-gradient(135deg, #f5f5f5 0%, #e8e8e8 100%);
            --bg-secondary: rgba(255, 255, 255, 0.9);
            --bg-card: rgba(255, 255, 255, 0.8);
            --bg-card-hover: rgba(0, 200, 255, 0.15);
            --border-primary: rgba(0, 0, 0, 0.1);
            --border-accent: rgba(0, 200, 255, 0.3);
            --text-primary: #1a1a1a;
            --text-secondary: #555555;
            --text-muted: #888888;
            --accent-primary: #0099cc;
            --accent-secondary: #ff6b35;
            --shadow-glow: rgba(0, 200, 255, 0.2);
        }

        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Inter', sans-serif;
            background: var(--bg-primary);
            color: var(--text-primary);
            min-height: 100vh;
            transition: all 0.3s ease;
        }

        .login-container { position: fixed; top: 0; left: 0; width: 100%; height: 100%;
            background: var(--bg-primary); display: flex; align-items: center; justify-content: center;
            z-index: 1000; transition: opacity 0.5s ease; }
        .login-container.hidden { opacity: 0; pointer-events: none; }
        .login-box {
            background: var(--bg-secondary); backdrop-filter: blur(20px);
            border: 1px solid var(--border-accent); border-radius: 20px; padding: 40px;
            width: 100%; max-width: 400px; box-shadow: 0 20px 40px rgba(0, 0, 0, 0.3);
            animation: fadeInUp 0.6s ease;
        }
        @keyframes fadeInUp { from { opacity: 0; transform: translateY(30px); } to { opacity: 1; transform: translateY(0); } }
        .login-header { text-align: center; margin-bottom: 30px; }
        .robot-icon {
            width: 80px; height: 80px; background: linear-gradient(45deg, var(--accent-secondary), #f7931e);
            border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 40px;
            margin: 0 auto 20px; box-shadow: 0 10px 30px rgba(255, 107, 53, 0.3);
        }
        .login-title { font-size: 28px; font-weight: 700; color: var(--accent-primary); margin-bottom: 8px; }
        .login-subtitle { color: var(--text-secondary); font-size: 14px; }

        .form-group { margin-bottom: 20px; }
        .form-group label { display: block; margin-bottom: 8px; color: var(--text-primary); font-weight: 500; font-size: 14px; }
        .input-group { position: relative; }
        .form-input {
            width: 100%; padding: 15px 20px; background: var(--bg-card); border: 1px solid var(--border-primary);
            border-radius: 12px; color: var(--text-primary); font-size: 16px; transition: all 0.3s ease;
        }
        .form-input:focus { outline: none; border-color: var(--accent-primary); box-shadow: 0 0 0 3px var(--shadow-glow); }
        .input-icon { position: absolute; right: 15px; top: 50%; transform: translateY(-50%); color: var(--text-muted); cursor: pointer; transition: color 0.3s ease; }
        .input-icon:hover { color: var(--accent-primary); }

        .login-btn {
            width: 100%; padding: 15px; background: linear-gradient(45deg, var(--accent-primary), var(--accent-secondary));
            border: none; border-radius: 12px; color: var(--text-primary); font-size: 16px; font-weight: 600;
            cursor: pointer; transition: all 0.3s ease; margin-top: 10px;
        }
        .login-btn:hover { transform: translateY(-2px); box-shadow: 0 10px 30px rgba(0, 255, 255, 0.3); }
        .login-btn:disabled { opacity: 0.6; cursor: not-allowed; transform: none; }

        .alert { padding: 12px 16px; border-radius: 8px; margin-bottom: 20px; font-size: 14px; display: none; }
        .alert-success { background: rgba(0, 255, 136, 0.1); border: 1px solid var(--success-color); color: var(--success-color); }
        .alert-error { background: rgba(255, 68, 68, 0.1); border: 1px solid var(--error-color); color: var(--error-color); }
        .loading {
            display: inline-block; width: 20px; height: 20px; border: 2px solid rgba(255, 255, 255, 0.3);
            border-radius: 50%; border-top-color: var(--text-primary); animation: spin 1s ease-in-out infinite; margin-right: 8px;
        }
        @keyframes spin { to { transform: rotate(360deg); } }

        .demo-credentials { margin-top: 20px; padding: 15px; background: var(--bg-card); border-radius: 8px; border: 1px solid var(--border-primary); }
        .demo-credentials h4 { color: var(--accent-primary); margin-bottom: 10px; font-size: 14px; }
        .demo-credentials p { color: var(--text-secondary); font-size: 12px; margin-bottom: 5px; }
        .demo-credentials strong { color: var(--text-primary); }

        .main-app { display: none; min-height: 100vh; }
        .main-app.active { display: block; }

        .container { max-width: 1440px; margin: 0 auto; padding: 0 16px; }
        .header { background: var(--bg-secondary); backdrop-filter: blur(10px); border-bottom: 1px solid var(--border-accent); padding: 16px 0; position: sticky; top: 0; z-index: 100; }
        .header-content { display: flex; justify-content: space-between; align-items: center; }
        .logo { display: flex; align-items: center; gap: 12px; font-family: 'Orbitron', monospace; font-weight: bold; font-size: 24px; color: var(--accent-primary); }
        .dragon-icon { width: 40px; height: 40px; background: linear-gradient(45deg, var(--accent-secondary), #f7931e); border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 20px; }

        .nav-section { display: flex; align-items: center; gap: 24px; }
        .nav-buttons { display: flex; gap: 12px; }
        .nav-btn {
            background: rgba(0, 255, 255, 0.1); border: 1px solid var(--border-accent); color: var(--accent-primary);
            padding: 8px 16px; border-radius: 8px; cursor: pointer; transition: all 0.3s ease; text-decoration: none; font-size: 14px;
        }
        .nav-btn:hover { background: var(--bg-card-hover); transform: translateY(-2px); }
        .nav-btn.active { background: var(--accent-primary); color: var(--bg-secondary); }
        .user-info { color: var(--text-secondary); margin-right: 15px; font-size: 14px; }
        .logout-btn { background: rgba(255, 68, 68, 0.1); border-color: #ff4444; color: #ff4444; }

        .main-content { padding: 24px 0; }
        .page { display: none; }
        .page.active { display: block; }

        .dashboard-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(400px, 1fr)); gap: 24px; }
        .control-card { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: 16px; padding: 24px; backdrop-filter: blur(10px); }
        .card-title { display: flex; align-items: center; gap: 12px; font-size: 20px; font-weight: 600; margin-bottom: 20px; color: var(--accent-primary); }
        .status-indicator { width: 12px; height: 12px; border-radius: 50%; animation: pulse 2s infinite; }
        .status-online { background: var(--success-color); }
        @keyframes pulse { 0%, 100% { opacity: 1; } 50% { opacity: 0.5; } }

        .vehicle-controls { display: grid; grid-template-columns: repeat(3, 1fr); gap: 12px; margin-bottom: 20px; }
        .direction-btn {
            background: var(--bg-card); border: 1px solid var(--border-primary); color: var(--text-primary);
            padding: 16px; border-radius: 12px; cursor: pointer; transition: all 0.3s ease; font-size: 18px; font-weight: bold;
        }
        .direction-btn:hover { background: var(--bg-card-hover); transform: translateY(-2px); }
        .stop-btn { background: var(--error-color); color: white; }

        .slider-group { margin-bottom: 16px; }
        .slider-label { display: flex; justify-content: space-between; align-items: center; margin-bottom: 8px; font-size: 14px; color: var(--text-secondary); }
        .slider { width: 100%; height: 8px; background: var(--border-primary); border-radius: 4px; outline: none; -webkit-appearance: none; }
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none; width: 20px; height: 20px; background: var(--accent-primary); border-radius: 50%; cursor: pointer;
        }
        .slider::-moz-range-thumb { width: 20px; height: 20px; background: var(--accent-primary); border-radius: 50%; cursor: pointer; border: none; }

        #arm3d-canvas { width: 100%; height: 500px; border-radius: 12px; background: var(--bg-card); display: block; }
        .arm-controls-panel { background: var(--bg-card); border: 1px solid var(--border-primary); border-radius: 16px; padding: 24px; overflow-y: auto; }
        .joint-group { margin-bottom: 24px; padding: 16px; background: var(--bg-secondary); border-radius: 12px; border: 1px solid var(--border-accent); }
        .joint-title { font-size: 16px; font-weight: 600; color: var(--accent-primary); margin-bottom: 12px; }
        .angle-display { background: var(--bg-primary); padding: 8px 12px; border-radius: 6px; font-family: 'Courier New', monospace; text-align: center; margin-top: 8px; border: 1px solid var(--border-primary); }

        /* Toggle Switch Styles */
        .toggle-container { display: flex; align-items: center; gap: 12px; margin-bottom: 16px; }
        .toggle-label { font-size: 14px; color: var(--text-secondary); font-weight: 500; }
        .toggle-switch { position: relative; display: inline-block; width: 60px; height: 34px; }
        .toggle-switch input { opacity: 0; width: 0; height: 0; }
        .toggle-slider {
            position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0;
            background-color: var(--border-primary); transition: 0.3s; border-radius: 34px;
        }
        .toggle-slider:before {
            position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px;
            background-color: white; transition: 0.3s; border-radius: 50%;
        }
        .toggle-switch input:checked + .toggle-slider { background-color: var(--accent-primary); }
        .toggle-switch input:checked + .toggle-slider:before { transform: translateX(26px); }
        .toggle-switch input:disabled + .toggle-slider { opacity: 0.5; cursor: not-allowed; }

        /* Theme Toggle Button */
        .theme-toggle {
            width: 48px; height: 48px; border-radius: 50%;
            background: var(--bg-card); border: 2px solid var(--border-accent);
            color: var(--accent-primary); cursor: pointer;
            transition: all 0.3s ease; display: flex; align-items: center; justify-content: center;
            font-size: 24px; position: relative;
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.2);
            line-height: 1;
            padding: 0;
        }
        .theme-toggle:hover {
            background: var(--bg-card-hover);
            transform: translateY(-2px) scale(1.05);
            box-shadow: 0 4px 12px rgba(0, 255, 255, 0.3);
            border-color: var(--accent-primary);
        }
        .theme-toggle:active { transform: translateY(0) scale(0.95); }
        .theme-toggle i,
        .theme-toggle .theme-icon {
            transition: all 0.4s cubic-bezier(0.68, -0.55, 0.265, 1.55);
            display: inline-block;
            font-size: 24px;
            line-height: 1;
            width: 24px;
            height: 24px;
        }
        .theme-toggle:hover i,
        .theme-toggle:hover .theme-icon {
            transform: rotate(360deg) scale(1.1);
        }
        body.light-mode .theme-toggle {
            color: #ffa500;
        }
        body.light-mode .theme-toggle:hover {
            box-shadow: 0 4px 12px rgba(255, 165, 0, 0.4);
            border-color: #ffa500;
        }

        @media (max-width: 768px) {
            .dashboard-grid { grid-template-columns: 1fr; }
            .nav-buttons { display: none; }
            .login-box { margin: 20px; padding: 30px 20px; }
        }
    </style>
    
</head>
<body>
    <div class="login-container" id="loginContainer">
        <div class="login-box">
            <div class="login-header">
                <div class="robot-icon">🤖</div>
                <h1 class="login-title">ESP32 Robot Control</h1>
                <p class="login-subtitle">Access the Robot Control Panel</p>
            </div>
            <div id="alertMessage" class="alert"></div>
            <form id="loginForm">
                <div class="form-group">
                    <label for="username">Username</label>
                    <div class="input-group">
                        <input type="text" id="username" class="form-input" placeholder="Enter your username" required>
                        <i class="fas fa-user input-icon"></i>
                    </div>
                </div>
                <div class="form-group">
                    <label for="password">Password</label>
                    <div class="input-group">
                        <input type="password" id="password" class="form-input" placeholder="Enter your password" required>
                        <i class="fas fa-eye input-icon" id="togglePassword"></i>
                    </div>
                </div>
                <button type="submit" class="login-btn" id="loginBtn">
                    <span id="loginText">Sign In</span>
                    <span id="loginSpinner" class="loading" style="display: none;"></span>
                </button>
            </form>
            <div class="demo-credentials">
                <h4><i class="fas fa-info-circle"></i> Demo Credentials</h4>
                <p><strong>Username:</strong> harendra</p>
                <p><strong>Password:</strong> 12345678</p>
            </div>
        </div>
    </div>

    <div class="main-app" id="mainApp">
        <header class="header">
            <div class="container">
                <div class="header-content">
                    <div class="logo">
                        <div class="dragon-icon">🐉</div>
                        <span>ESP32 ROBOT</span>
                    </div>
                    <div class="nav-section">
                        <nav class="nav-buttons">
                            <a href="#" class="nav-btn active" data-page="dashboard">Dashboard</a>
                            <a href="#" class="nav-btn" data-page="arm3d">3D Arm Control</a>
                        </nav>
                        <button class="theme-toggle" id="themeToggle" title="Toggle Light/Dark Mode">
                            <span class="theme-icon" id="themeIcon">🌙</span>
                        </button>
                        <div class="user-info" id="userInfo">Welcome, User</div>
                        <button class="nav-btn logout-btn" onclick="logout()">
                            <i class="fas fa-sign-out-alt"></i> Logout
                        </button>
                    </div>
                </div>
            </div>
        </header>

        <main class="container">
            <div class="main-content">
                <div id="dashboard" class="page active">
                    <div class="dashboard-grid">
                        <div class="control-card">
                            <h2 class="card-title">
                                <span class="status-indicator status-online"></span>
                                Vehicle Control
                            </h2>
                            <div class="vehicle-controls">
                                <div></div>
                                <button class="direction-btn" data-direction="forward">↑</button>
                                <div></div>
                                <button class="direction-btn" data-direction="left">←</button>
                                <button class="direction-btn stop-btn" data-direction="stop">STOP</button>
                                <button class="direction-btn" data-direction="right">→</button>
                                <div></div>
                                <button class="direction-btn" data-direction="backward">↓</button>
                                <div></div>
                            </div>
                            <div class="slider-group">
                                <div class="slider-label">
                                    <span>Speed Control</span>
                                    <span id="speed-value">50%</span>
                                </div>
                                <input type="range" class="slider" id="speed-slider" min="0" max="100" value="50">
                            </div>
                            <div class="toggle-container">
                                <span class="toggle-label"><i class="fas fa-lightbulb"></i> Light</span>
                                <label class="toggle-switch">
                                    <input type="checkbox" id="light-toggle">
                                    <span class="toggle-slider"></span>
                                </label>
                            </div>
                        </div>

                        <div class="control-card">
                            <h2 class="card-title">
                                <span class="status-indicator status-online"></span>
                                Robotic Arm Control
                            </h2>
                            <div class="slider-group">
                                <div class="slider-label">
                                    <span>Base Rotation</span>
                                    <span id="base-value">90°</span>
                                </div>
                                <input type="range" class="slider" id="base-slider" min="0" max="180" value="90">
                            </div>
                            <div class="slider-group">
                                <div class="slider-label">
                                    <span>Shoulder</span>
                                    <span id="shoulder-value">0°</span>
                                </div>
                                <input type="range" class="slider" id="shoulder-slider" min="-90" max="90" value="0">
                            </div>
                            <div class="slider-group">
                                <div class="slider-label">
                                    <span>Elbow</span>
                                    <span id="elbow-value">0°</span>
                                </div>
                                <input type="range" class="slider" id="elbow-slider" min="-90" max="90" value="0">
                            </div>
                            <div class="slider-group">
                                <div class="slider-label">
                                    <span>Wrist</span>
                                    <span id="wrist-value">0°</span>
                                </div>
                                <input type="range" class="slider" id="wrist-slider" min="-90" max="90" value="0">
                            </div>
                            <div class="slider-group">
                                <div class="slider-label">
                                    <span>Gripper</span>
                                    <span id="gripper-value">0°</span>
                                </div>
                                <input type="range" class="slider" id="gripper-slider" min="0" max="45" value="0">
                            </div>
                        </div>
                    </div>
                </div>

                <div id="arm3d" class="page">
                    <div style="display: grid; grid-template-columns: 2fr 1fr; gap: 24px;">
                        <div style="background: var(--bg-card); border-radius: 16px; padding: 16px;">
                            <canvas id="arm3d-canvas"></canvas>
                        </div>
                        <div class="arm-controls-panel">
                            <h2 class="card-title">Joint Controls</h2>
                            <div class="joint-group">
                                <div class="joint-title">Base Joint (0°-180°)</div>
                                <input type="range" class="slider" id="joint1-slider" min="0" max="180" value="0">
                                <div class="angle-display" id="joint1-display">0°</div>
                            </div>
                            <div class="joint-group">
                                <div class="joint-title">Shoulder Joint (-90° to 90°)</div>
                                <input type="range" class="slider" id="joint2-slider" min="-90" max="90" value="0">
                                <div class="angle-display" id="joint2-display">0°</div>
                            </div>
                            <div class="joint-group">
                                <div class="joint-title">Elbow Joint (-90° to 90°)</div>
                                <input type="range" class="slider" id="joint3-slider" min="-90" max="90" value="0">
                                <div class="angle-display" id="joint3-display">0°</div>
                            </div>
                            <div class="joint-group">
                                <div class="joint-title">Wrist Pitch (-90° to 90°)</div>
                                <input type="range" class="slider" id="joint4-slider" min="-90" max="90" value="0">
                                <div class="angle-display" id="joint4-display">0°</div>
                            </div>
                            <div class="joint-group">
                                <div class="joint-title">Wrist Roll (-180° to 180°)</div>
                                <input type="range" class="slider" id="joint5-slider" min="-180" max="180" value="0">
                                <div class="angle-display" id="joint5-display">0°</div>
                            </div>
                            <div class="joint-group">
                                <div class="joint-title">Gripper (0° to 45°)</div>
                                <input type="range" class="slider" id="joint6-slider" min="0" max="45" value="0">
                                <div class="angle-display" id="joint6-display">0°</div>
                            </div>
                            <div style="margin-top: 24px; display: flex; gap: 12px;">
                                <button class="nav-btn" id="reset-arm">Reset Position</button>
                                <button class="nav-btn" id="home-arm">Home Position</button>
                                <button class="nav-btn" id="demo-arm">Demo Sequence</button>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </main>
    </div>

    <script>
        let scene, camera, renderer, controls;
        let armJoints = [];
        let armGroup;
        let esp32IP = '%ESP32_IP%';
        let animationId = null;
        let isAnimating = false;

        const VALID_USERNAME = 'harendra';
        const VALID_PASSWORD = '12345678';

        const loginContainer = document.getElementById('loginContainer');
        const mainApp = document.getElementById('mainApp');
        const loginForm = document.getElementById('loginForm');
        const usernameInput = document.getElementById('username');
        const passwordInput = document.getElementById('password');
        const togglePassword = document.getElementById('togglePassword');
        const loginBtn = document.getElementById('loginBtn');
        const loginText = document.getElementById('loginText');
        const loginSpinner = document.getElementById('loginSpinner');
        const alertMessage = document.getElementById('alertMessage');
        const userInfo = document.getElementById('userInfo');

        function checkAuth() {
            const isLoggedIn = localStorage.getItem('robotLoggedIn');
            const username = localStorage.getItem('robotUsername');
            if (isLoggedIn === 'true' && username) {
                showMainApp(username);
                return true;
            }
            return false;
        }

        function showMainApp(username) {
            loginContainer.classList.add('hidden');
            mainApp.classList.add('active');
            userInfo.textContent = `Welcome, ${username}`;
            initializeApp();
        }

        function showLogin() {
            loginContainer.classList.remove('hidden');
            mainApp.classList.remove('active');
        }

        togglePassword.addEventListener('click', function() {
            const type = passwordInput.getAttribute('type') === 'password' ? 'text' : 'password';
            passwordInput.setAttribute('type', type);
            togglePassword.classList.toggle('fa-eye');
            togglePassword.classList.toggle('fa-eye-slash');
        });

        function showAlert(message, type = 'error') {
            alertMessage.textContent = message;
            alertMessage.className = `alert alert-${type}`;
            alertMessage.style.display = 'block';
            setTimeout(() => { alertMessage.style.display = 'none'; }, 5000);
        }

        function setLoading(loading) {
            loginBtn.disabled = loading;
            loginText.style.display = loading ? 'none' : 'inline';
            loginSpinner.style.display = loading ? 'inline-block' : 'none';
        }

        loginForm.addEventListener('submit', async function(e) {
            e.preventDefault();
            const username = usernameInput.value.trim();
            const password = passwordInput.value.trim();
            if (!username || !password) {
                showAlert('Please enter both username and password');
                return;
            }
            setLoading(true);
            await new Promise(resolve => setTimeout(resolve, 1000));
            if (username === VALID_USERNAME && password === VALID_PASSWORD) {
                showAlert('Login successful!', 'success');
                localStorage.setItem('robotLoggedIn', 'true');
                localStorage.setItem('robotUsername', username);
                setTimeout(() => { showMainApp(username); }, 1500);
            } else {
                showAlert('Invalid username or password. Please try again.');
                setLoading(false);
            }
        });

        function logout() {
            if (confirm('Are you sure you want to logout?')) {
                localStorage.removeItem('robotLoggedIn');
                localStorage.removeItem('robotUsername');
                showLogin();
            }
        }

        function initializeApp() {
            const navButtons = document.querySelectorAll('.nav-btn');
            const pages = document.querySelectorAll('.page');
            navButtons.forEach(btn => {
                btn.addEventListener('click', (e) => {
                    e.preventDefault();
                    const targetPage = btn.dataset.page;
                    if (targetPage) {
                        navButtons.forEach(b => b.classList.remove('active'));
                        btn.classList.add('active');
                        pages.forEach(page => page.classList.remove('active'));
                        document.getElementById(targetPage).classList.add('active');
                        if (targetPage === 'arm3d' && typeof init3DArm === 'function') {
                            setTimeout(init3DArm, 100);
                        }
                    }
                });
            });

            const directionBtns = document.querySelectorAll('.direction-btn');
            directionBtns.forEach(btn => {
                btn.addEventListener('click', () => {
                    const direction = btn.dataset.direction;
                    fetch(`http://${esp32IP}/move?direction=${direction}`)
                        .then(response => response.text())
                        .then(data => console.log(data))
                        .catch(error => console.error('Error:', error));
                    btn.style.background = 'var(--accent-primary)';
                    btn.style.color = 'var(--bg-secondary)';
                    setTimeout(() => {
                        btn.style.background = '';
                        btn.style.color = '';
                    }, 200);
                });
            });

            const sliders = {
                'speed-slider': 'speed-value',
                'base-slider': 'base-value',
                'shoulder-slider': 'shoulder-value',
                'elbow-slider': 'elbow-value',
                'wrist-slider': 'wrist-value',
                'gripper-slider': 'gripper-value'
            };

            Object.entries(sliders).forEach(([sliderId, valueId]) => {
                const slider = document.getElementById(sliderId);
                const valueDisplay = document.getElementById(valueId);
                if (slider && valueDisplay) {
                    slider.addEventListener('input', (e) => {
                        const value = e.target.value;
                        const unit = valueId.includes('speed') ? '%' : '°';
                        valueDisplay.textContent = value + unit;
                        let command = '';
                        if (sliderId === 'speed-slider') {
                            command = `SPEED:${value}`;
                        } else {
                            let servoNum;
                            switch (sliderId) {
                                case 'base-slider': servoNum = 1; break;
                                case 'shoulder-slider': servoNum = 2; break;
                                case 'elbow-slider': servoNum = 3; break;
                                case 'wrist-slider': servoNum = 4; break;
                                case 'gripper-slider': servoNum = 6; break;
                            }
                            command = `SERVO:${servoNum}:${value}`;
                        }
                        fetch(`http://${esp32IP}/command?cmd=${command}`)
                            .then(response => response.text())
                            .then(data => console.log(data))
                            .catch(error => console.error('Error:', error));
                    });
                }
            });

            initialize3DControls();
            
            // Initialize theme toggle
            initializeThemeToggle();
            
            // Initialize light toggle
            initializeLightToggle();
        }
        
        function initializeThemeToggle() {
            const themeToggle = document.getElementById('themeToggle');
            const themeIcon = document.getElementById('themeIcon');
            
            // Load saved theme preference
            const savedTheme = localStorage.getItem('robotTheme') || 'dark';
            if (savedTheme === 'light') {
                document.body.classList.add('light-mode');
                themeIcon.textContent = '☀️';
            } else {
                themeIcon.textContent = '🌙';
            }
            
            themeToggle.addEventListener('click', () => {
                const isLightMode = document.body.classList.toggle('light-mode');
                if (isLightMode) {
                    themeIcon.textContent = '☀️';
                    localStorage.setItem('robotTheme', 'light');
                } else {
                    themeIcon.textContent = '🌙';
                    localStorage.setItem('robotTheme', 'dark');
                }
            });
        }
        
        function initializeLightToggle() {
            const lightToggle = document.getElementById('light-toggle');
            
            if (!lightToggle) return;
            
            // Load saved light state
            const savedLightState = localStorage.getItem('robotLightState') === 'true';
            lightToggle.checked = savedLightState;
            
            lightToggle.addEventListener('change', function() {
                const state = this.checked ? 'on' : 'off';
                fetch(`http://${esp32IP}/light?state=${state}`)
                    .then(response => response.text())
                    .then(data => {
                        console.log(`Light turned ${state}:`, data);
                        localStorage.setItem('robotLightState', this.checked.toString());
                    })
                    .catch(error => {
                        console.error('Error controlling light:', error);
                        // Revert toggle on error
                        this.checked = !this.checked;
                    });
            });
        }

        function initialize3DControls() {
            const jointSliders = [
                'joint1-slider', 'joint2-slider', 'joint3-slider',
                'joint4-slider', 'joint5-slider', 'joint6-slider'
            ];
            jointSliders.forEach((sliderId, index) => {
                const slider = document.getElementById(sliderId);
                const display = document.getElementById(sliderId.replace('slider', 'display'));
                if (slider && display) {
                    slider.addEventListener('input', (e) => {
                        const value = parseFloat(e.target.value);
                        const unit = '°';
                        display.textContent = value + unit;
                        const servoNum = index === 5 ? 6 : (index + 1);
                        fetch(`http://${esp32IP}/command?cmd=SERVO:${servoNum}:${value}`)
                            .then(response => response.text())
                            .then(data => console.log(data))
                            .catch(error => console.error('Error:', error));
                        updateArmJoint(index, value);
                    });
                }
            });

            document.getElementById('reset-arm').addEventListener('click', () => {
                fetch(`http://${esp32IP}/command?cmd=RESET`)
                    .then(response => response.text())
                    .then(data => console.log(data))
                    .catch(error => console.error('Error:', error));
                resetArmPosition();
            });

            document.getElementById('home-arm').addEventListener('click', () => {
                fetch(`http://${esp32IP}/command?cmd=HOME`)
                    .then(response => response.text())
                    .then(data => console.log(data))
                    .catch(error => console.error('Error:', error));
                homeArmPosition();
            });

            document.getElementById('demo-arm').addEventListener('click', () => {
                fetch(`http://${esp32IP}/command?cmd=DEMO`)
                    .then(response => response.text())
                    .then(data => console.log(data))
                    .catch(error => console.error('Error:', error));
                playArmSequence();
            });
        }

        function init3DArm() {
            const canvas = document.getElementById('arm3d-canvas');
            const container = canvas.parentElement;
            if (!container || renderer) return;

            scene = new THREE.Scene();
            scene.background = new THREE.Color(0x1a1a2e);

            camera = new THREE.PerspectiveCamera(75, container.clientWidth / container.clientHeight, 0.1, 1000);
            camera.position.set(5, 5, 5);
            camera.lookAt(0, 0, 0);

            renderer = new THREE.WebGLRenderer({ canvas: canvas, antialias: true });
            renderer.setSize(container.clientWidth, container.clientHeight);
            renderer.shadowMap.enabled = true;
            renderer.shadowMap.type = THREE.PCFSoftShadowMap;

            const ambientLight = new THREE.AmbientLight(0x404040, 0.6);
            scene.add(ambientLight);
            const directionalLight = new THREE.DirectionalLight(0xffffff, 0.8);
            directionalLight.position.set(10, 10, 5);
            directionalLight.castShadow = true;
            directionalLight.shadow.mapSize.width = 2048;
            directionalLight.shadow.mapSize.height = 2048;
            scene.add(directionalLight);

            createRoboticArm();

            controls = new THREE.OrbitControls(camera, renderer.domElement);
            controls.enableDamping = true;
            controls.dampingFactor = 0.05;

            const gridHelper = new THREE.GridHelper(10, 10);
            scene.add(gridHelper);

            function animate() {
                requestAnimationFrame(animate);
                controls.update();
                renderer.render(scene, camera);
            }
            animate();

            window.addEventListener('resize', () => {
                const width = container.clientWidth;
                const height = container.clientHeight;
                camera.aspect = width / height;
                camera.updateProjectionMatrix();
                renderer.setSize(width, height);
            });
        }

        function createRoboticArm() {
            armGroup = new THREE.Group();
            scene.add(armGroup);

            const baseColor = 0x666666;

            const baseGeometry = new THREE.CylinderGeometry(1, 1.2, 0.5, 16);
            const baseMaterial = new THREE.MeshPhongMaterial({ color: baseColor });
            const base = new THREE.Mesh(baseGeometry, baseMaterial);
            base.position.y = 0.25;
            base.castShadow = true;
            armGroup.add(base);

            const joint1Group = new THREE.Group();
            base.add(joint1Group);

            const link1Geometry = new THREE.BoxGeometry(0.3, 2, 0.3);
            const link1Material = new THREE.MeshPhongMaterial({ color: 0x00ffff });
            const link1 = new THREE.Mesh(link1Geometry, link1Material);
            link1.position.y = 1.25;
            link1.castShadow = true;
            joint1Group.add(link1);

            const joint2Group = new THREE.Group();
            joint2Group.position.y = 2.25;
            joint1Group.add(joint2Group);

            const link2Geometry = new THREE.BoxGeometry(0.25, 1.5, 0.25);
            const link2Material = new THREE.MeshPhongMaterial({ color: 0xff6b35 });
            const link2 = new THREE.Mesh(link2Geometry, link2Material);
            link2.position.y = 0.75;
            link2.castShadow = true;
            joint2Group.add(link2);

            const joint3Group = new THREE.Group();
            joint3Group.position.y = 1.5;
            joint2Group.add(joint3Group);

            const link3Geometry = new THREE.BoxGeometry(0.2, 1.2, 0.2);
            const link3Material = new THREE.MeshPhongMaterial({ color: 0x00ffff });
            const link3 = new THREE.Mesh(link3Geometry, link3Material);
            link3.position.y = 0.6;
            link3.castShadow = true;
            joint3Group.add(link3);

            const joint4Group = new THREE.Group();
            joint4Group.position.y = 1.2;
            joint3Group.add(joint4Group);

            const link4Geometry = new THREE.BoxGeometry(0.15, 0.8, 0.15);
            const link4Material = new THREE.MeshPhongMaterial({ color: 0xff6b35 });
            const link4 = new THREE.Mesh(link4Geometry, link4Material);
            link4.position.y = 0.4;
            link4.castShadow = true;
            joint4Group.add(link4);

            const joint5Group = new THREE.Group();
            joint5Group.position.y = 0.8;
            joint4Group.add(joint5Group);

            const gripperGeometry = new THREE.BoxGeometry(0.1, 0.3, 0.1);
            const gripperMaterial = new THREE.MeshPhongMaterial({ color: 0x00ff00 });
            const gripper = new THREE.Mesh(gripperGeometry, gripperMaterial);
            gripper.position.y = 0.15;
            gripper.castShadow = true;
            joint5Group.add(gripper);

            window.armJoints = [joint1Group, joint2Group, joint3Group, joint4Group, joint5Group];
        }

        function updateArmJoint(jointIndex, angle) {
            if (!window.armJoints || !window.armJoints[jointIndex]) return;
            const radians = (angle * Math.PI) / 180;
            const joint = window.armJoints[jointIndex];
            switch (jointIndex) {
                case 0: joint.rotation.y = radians; break;
                case 1: joint.rotation.z = radians; break;
                case 2: joint.rotation.z = radians; break;
                case 3: joint.rotation.z = radians; break;
                case 4: joint.rotation.y = radians; break;
                case 5:
                    const scale = 1 + (angle / 100) * 0.5;
                    joint.scale.x = scale;
                    joint.scale.z = scale;
                    break;
            }
        }

        function resetArmPosition() {
            const sliders = ['joint1-slider', 'joint2-slider', 'joint3-slider', 'joint4-slider', 'joint5-slider', 'joint6-slider'];
            sliders.forEach((sliderId, index) => {
                const slider = document.getElementById(sliderId);
                if (slider) {
                    slider.value = 0;
                    const display = document.getElementById(sliderId.replace('slider', 'display'));
                    if (display) {
                        const unit = '°';
                        display.textContent = '0' + unit;
                    }
                    updateArmJoint(index, 0);
                }
            });
        }

        function homeArmPosition() {
            const homePositions = [0, 0, 0, 0, 0, 0];
            const sliders = ['joint1-slider', 'joint2-slider', 'joint3-slider', 'joint4-slider', 'joint5-slider', 'joint6-slider'];
            sliders.forEach((sliderId, index) => {
                const slider = document.getElementById(sliderId);
                if (slider) {
                    slider.value = homePositions[index];
                    const display = document.getElementById(sliderId.replace('slider', 'display'));
                    if (display) {
                        const unit = '°';
                        display.textContent = homePositions[index] + unit;
                    }
                    updateArmJoint(index, homePositions[index]);
                }
            });
        }

        function playArmSequence() {
            if (isAnimating) return;

            const sequences = [
                { joint: 0, target: 90, duration: 1000 },
                { joint: 1, target: -30, duration: 1000 },
                { joint: 2, target: 45, duration: 1000 },
                { joint: 3, target: -20, duration: 1000 },
                { joint: 4, target: 45, duration: 1000 },
                { joint: 5, target: 45, duration: 500 },
                { joint: 5, target: 22, duration: 500 },
                { joint: 5, target: 45, duration: 500 },
                { joint: 5, target: 22, duration: 500 },
                { joint: 0, target: 0, duration: 1000 },
                { joint: 1, target: 0, duration: 1000 },
                { joint: 2, target: 0, duration: 1000 },
                { joint: 3, target: 0, duration: 1000 },
                { joint: 4, target: 0, duration: 1000 },
                { joint: 5, target: 0, duration: 500 }
            ];

            let currentStep = 0;
            isAnimating = true;

            function animateStep() {
                if (currentStep >= sequences.length) {
                    isAnimating = false;
                    return;
                }
                const step = sequences[currentStep];
                const slider = document.getElementById(`joint${step.joint + 1}-slider`);
                const display = document.getElementById(`joint${step.joint + 1}-display`);
                if (slider && display) {
                    const startValue = parseFloat(slider.value);
                    const valueChange = step.target - startValue;
                    const startTime = performance.now();
                    const unit = '°';
                    function updateValue(timestamp) {
                        const elapsed = timestamp - startTime;
                        const progress = Math.min(elapsed / step.duration, 1);
                        const currentValue = startValue + (valueChange * progress);
                        slider.value = currentValue;
                        display.textContent = Math.round(currentValue) + unit;
                        updateArmJoint(step.joint, currentValue);
                        if (progress < 1) {
                            animationId = requestAnimationFrame(updateValue);
                        } else {
                            currentStep++;
                            animateStep();
                        }
                    }
                    animationId = requestAnimationFrame(updateValue);
                } else {
                    currentStep++;
                    animateStep();
                }
            }
            animateStep();
        }

        document.addEventListener('keypress', function(e) {
            if (e.key === 'Enter' && !loginContainer.classList.contains('hidden')) {
                loginForm.dispatchEvent(new Event('submit'));
            }
        });

        window.addEventListener('load', function() {
            // Apply saved theme on load
            const savedTheme = localStorage.getItem('robotTheme') || 'dark';
            if (savedTheme === 'light') {
                document.body.classList.add('light-mode');
                const themeIcon = document.getElementById('themeIcon');
                if (themeIcon) {
                    themeIcon.textContent = '☀️';
                }
            } else {
                const themeIcon = document.getElementById('themeIcon');
                if (themeIcon) {
                    themeIcon.textContent = '🌙';
                }
            }
            
            if (!checkAuth()) {
                usernameInput.focus();
            }
        });
    </script>
</body>
</html>
)=====";

