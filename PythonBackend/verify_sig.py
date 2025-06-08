from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives.asymmetric.utils import Prehashed
import base64
import hashlib

# 加载公钥
with open("public_key.pem", "rb") as key_file:
    public_key = serialization.load_pem_public_key(
        key_file.read(),
        backend=default_backend()
    )

# 读取原始文件
original_file = "example.txt"
with open(original_file, "rb") as f:
    file_data = f.read()

# 计算文件哈希
file_hash = hashlib.sha256(file_data).digest()

# 读取签名文件
signature_file = "example.txt.sig"
with open(signature_file, "rb") as f:
    signature = base64.b64decode(f.read())

# 验证签名
try:
    public_key.verify(
        signature,
        file_hash,
        padding.PKCS1v15(),
        Prehashed(hashes.SHA256())
    )
    print("签名验证成功！文件未被篡改。")
except:
    print("签名验证失败！文件可能已被篡改。")