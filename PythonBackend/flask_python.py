import os
import base64
import hashlib
from flask import Flask, request, send_file, render_template_string, redirect, url_for
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives.asymmetric.utils import Prehashed

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = 'uploads'
app.config['SIGNATURE_FOLDER'] = 'signatures'
app.config['KEYS_FOLDER'] = 'keys'
app.config['MAX_CONTENT_LENGTH'] = 1000 * 1024 * 1024  # 16MB 文件大小限制

# 创建必要的文件夹
os.makedirs(app.config['UPLOAD_FOLDER'], exist_ok=True)
os.makedirs(app.config['SIGNATURE_FOLDER'], exist_ok=True)
os.makedirs(app.config['KEYS_FOLDER'], exist_ok=True)

# 生成或加载 RSA 密钥对
def setup_keys():
    private_key_path = os.path.join(app.config['KEYS_FOLDER'], 'private_key.pem')
    public_key_path = os.path.join(app.config['KEYS_FOLDER'], 'public_key.pem')

    if os.path.exists(private_key_path) and os.path.exists(public_key_path):
        # 加载现有密钥
        with open(private_key_path, "rb") as key_file:
            private_key = serialization.load_pem_private_key(
                key_file.read(),
                password=None,
                backend=default_backend()
            )

        with open(public_key_path, "rb") as key_file:
            public_key = serialization.load_pem_public_key(
                key_file.read(),
                backend=default_backend()
            )
    else:
        # 生成新密钥对
        private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=2048,
            backend=default_backend()
        )
        public_key = private_key.public_key()

        # 保存私钥
        pem_private = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,
            encryption_algorithm=serialization.NoEncryption()
        )
        with open(private_key_path, "wb") as f:
            f.write(pem_private)

        # 保存公钥
        pem_public = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
        with open(public_key_path, "wb") as f:
            f.write(pem_public)

    return private_key, public_key

# 初始化密钥
private_key, public_key = setup_keys()

# HTML 模板
HTML_TEMPLATE = """
<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RSA 2048 文件签名服务</title>
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        body {
            background: linear-gradient(135deg, #1a2a6c, #b21f1f, #fdbb2d);
            color: #333;
            min-height: 100vh;
            padding: 20px;
            display: flex;
            justify-content: center;
            align-items: center;
        }
        .container {
            background-color: rgba(255, 255, 255, 0.92);
            border-radius: 15px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
            width: 100%;
            max-width: 800px;
            padding: 30px;
            margin: 20px;
        }
        header {
            text-align: center;
            margin-bottom: 30px;
            border-bottom: 2px solid #eee;
            padding-bottom: 20px;
        }
        h1 {
            color: #2c3e50;
            margin-bottom: 10px;
            font-size: 2.5rem;
        }
        .subtitle {
            color: #7f8c8d;
            font-size: 1.1rem;
        }
        .card {
            background: white;
            border-radius: 10px;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.08);
            padding: 25px;
            margin-bottom: 25px;
        }
        .card h2 {
            color: #3498db;
            margin-bottom: 20px;
            font-size: 1.5rem;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #2c3e50;
        }
        .file-input {
            width: 100%;
            padding: 12px;
            border: 2px dashed #3498db;
            border-radius: 8px;
            background-color: #f8f9fa;
            cursor: pointer;
            transition: all 0.3s;
            text-align: center;
        }
        .file-input:hover {
            background-color: #e3f2fd;
            border-color: #2980b9;
        }
        .btn {
            background: linear-gradient(to right, #3498db, #2c3e50);
            color: white;
            border: none;
            padding: 14px 28px;
            font-size: 1.1rem;
            border-radius: 8px;
            cursor: pointer;
            width: 100%;
            font-weight: 600;
            transition: all 0.3s;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 8px rgba(0, 0, 0, 0.15);
            background: linear-gradient(to right, #2980b9, #1a252f);
        }
        .btn:active {
            transform: translateY(0);
        }
        .message {
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
            text-align: center;
        }
        .success {
            background-color: #d4edda;
            color: #155724;
            border: 1px solid #c3e6cb;
        }
        .error {
            background-color: #f8d7da;
            color: #721c24;
            border: 1px solid #f5c6cb;
        }
        .info {
            background-color: #cce5ff;
            color: #004085;
            border: 1px solid #b8daff;
        }
        .key-info {
            display: flex;
            flex-direction: column;
            background-color: #e9ecef;
            border-left: 4px solid #3498db;
            padding: 15px;
            gap: 10px;
            margin: 20px 0;
            border-radius: 4px;
            align-items: start;
        }
        .key-info p {
            margin: 0;
        }
        .key-info .btn {
            display: inline-block;
            padding: 10px 20px;
            background-color: #007bff;
            color: white;
            text-decoration: none;
            border-radius: 5px;
        }
        footer {
            text-align: center;
            margin-top: 30px;
            padding-top: 20px;
            border-top: 1px solid #eee;
            color: #7f8c8d;
            font-size: 0.9rem;
        }
        .features {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin: 25px 0;
        }
        .feature {
            background: #f8f9fa;
            padding: 15px;
            border-radius: 8px;
            text-align: center;
            border: 1px solid #e9ecef;
        }
        .feature i {
            font-size: 2rem;
            color: #3498db;
            margin-bottom: 10px;
        }
        @media (max-width: 600px) {
            .container {
                padding: 15px;
            }
            h1 {
                font-size: 2rem;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>RSA 2048 文件签名服务</h1>
            <p class="subtitle">安全可靠的文件数字签名解决方案</p>
        </header>

        <div class="card">
            <h2>文件签名</h2>
            <form method="post" enctype="multipart/form-data">
                <div class="form-group">
                    <label for="file">选择要签名的文件：</label>
                    <input type="file" name="file" id="file" class="file-input" required>
                </div>
                <button type="submit" class="btn">生成签名</button>
            </form>

            {% if message %}
            <div class="message {{ message_class }}">{{ message }}</div>
            {% endif %}

            {% if signature_file %}
            <div class="key-info">
                <p>签名文件已生成！</p>
                <a href="{{ url_for('download_signature', filename=signature_file) }}" class="btn">下载签名文件</a>
            </div>
            {% endif %}
        </div>

        <div class="card">
            <h2>关于本服务</h2>
            <div class="key-info">
                <p><strong>签名算法：</strong> RSA 2048 with SHA-256</p>
                <p><strong>密钥类型：</strong> PKCS#8 格式</p>
                <p><strong>最大文件大小：</strong> 16 MB</p>
            </div>

            <div class="features">
                <div class="feature">
                    <h3>安全性</h3>
                    <p>使用行业标准 RSA 2048 算法</p>
                </div>
                <div class="feature">
                    <h3>可靠性</h3>
                    <p>SHA-256 哈希算法保证完整性</p>
                </div>
                <div class="feature">
                    <h3>便捷性</h3>
                    <p>一键上传下载操作</p>
                </div>
            </div>
        </div>

        <footer>
            <p>© 2023 RSA 文件签名服务 | 安全数字签名解决方案</p>
        </footer>
    </div>
</body>
</html>
"""

def generate_signature(file_path):
    """使用 RSA 2048 with SHA-256 生成文件签名"""
    # 计算文件的 SHA-256 哈希
    sha256 = hashlib.sha256()
    with open(file_path, 'rb') as f:
        while chunk := f.read(4096):
            sha256.update(chunk)
    file_hash = sha256.digest()

    # 使用私钥进行签名
    signature = private_key.sign(
        file_hash,
        padding.PKCS1v15(),
        Prehashed(hashes.SHA256())
    )

    return signature

@app.route('/', methods=['GET', 'POST'])
def index():
    message = None
    message_class = None
    signature_file = None

    if request.method == 'POST':
        # 检查文件是否上传
        if 'file' not in request.files:
            message = '没有选择文件'
            message_class = 'error'
        else:
            file = request.files['file']

            # 检查文件名是否为空
            if file.filename == '':
                message = '没有选择文件'
                message_class = 'error'
            else:
                try:
                    # 保存上传的文件
                    upload_path = os.path.join(app.config['UPLOAD_FOLDER'], file.filename)
                    file.save(upload_path)

                    # 生成签名
                    signature = generate_signature(upload_path)

                    # 保存签名到文件 (Base64 编码)
                    signature_filename = f"{file.filename}.sig"
                    signature_path = os.path.join(app.config['SIGNATURE_FOLDER'], signature_filename)

                    with open(signature_path, 'wb') as sig_file:
                        # 包含签名和原始文件名信息
                        sig_file.write(base64.b64encode(signature))

                    message = f"文件 '{file.filename}' 签名成功！"
                    message_class = 'success'
                    signature_file = signature_filename

                except Exception as e:
                    message = f'处理错误: {str(e)}'
                    message_class = 'error'

    return render_template_string(
        HTML_TEMPLATE,
        message=message,
        message_class=message_class,
        signature_file=signature_file
    )

@app.route('/download/<filename>')
def download_signature(filename):
    """下载签名文件"""
    signature_path = os.path.join(app.config['SIGNATURE_FOLDER'], filename)
    if not os.path.exists(signature_path):
        return "签名文件不存在", 404

    # 提取原始文件名用于下载文件名
    original_filename = filename[:-4]  # 移除 .sig 后缀

    return send_file(
        signature_path,
        as_attachment=True,
        download_name=f"{original_filename}_signature.sig",
        mimetype='application/octet-stream'
    )

@app.route('/public-key')
def get_public_key():
    """提供公钥下载"""
    public_key_path = os.path.join(app.config['KEYS_FOLDER'], 'public_key.pem')
    return send_file(
        public_key_path,
        as_attachment=True,
        download_name="public_key.pem",
        mimetype='application/x-pem-file'
    )

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)