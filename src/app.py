import os
import subprocess
import json
import zipfile
import datetime
import requests

from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS


# ============================================================
# KHỞI TẠO FLASK
# ============================================================

app = Flask(__name__)
CORS(app)


# ============================================================
# ĐƯỜNG DẪN
# ============================================================

# app.py nằm trong: D:\TL\FingerprintPlagiarism\src
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

# DATA nằm trong: D:\TL\FingerprintPlagiarism\data
DATA_DIR = os.path.abspath(os.path.join(BASE_DIR, '..', 'data'))

# Thư mục gốc project: D:\TL\FingerprintPlagiarism
PROJECT_DIR = os.path.abspath(os.path.join(BASE_DIR, '..'))

# detector.exe: D:\TL\FingerprintPlagiarism\src\detector.exe
EXE_PATH = os.path.join(BASE_DIR, 'detector.exe')

# dashboard.html: D:\TL\FingerprintPlagiarism\dashboard.html
DASHBOARD_PATH = os.path.join(PROJECT_DIR, 'dashboard.html')

# results.json mặc định
RESULTS_PATH = os.path.join(DATA_DIR, 'results.json')


# ============================================================
# IN THÔNG TIN SERVER
# ============================================================

print("=" * 70)
print("🚀 FINGERPRINT PLAGIARISM SERVER")
print("=" * 70)
print(f"📁 BASE_DIR    : {BASE_DIR}")
print(f"📁 PROJECT_DIR : {PROJECT_DIR}")
print(f"📁 DATA_DIR    : {DATA_DIR}")
print(f"⚙️  EXE_PATH    : {EXE_PATH}")
print(f"📄 RESULTS     : {RESULTS_PATH}")
print(f"🌐 DASHBOARD   : {DASHBOARD_PATH}")
print("=" * 70)


# ============================================================
# ROUTE TRANG CHỦ & DASHBOARD
# ============================================================

@app.route('/')
def index():
    return """
    <!DOCTYPE html>
    <html lang="vi">
    <head><meta charset="UTF-8"><title>Fingerprint Plagiarism</title></head>
    <body>
        <h2>Fingerprint Plagiarism Detection</h2>
        <p>Flask Server đang hoạt động.</p>
        <a href="/dashboard.html">Mở Dashboard</a>
    </body>
    </html>
    """

@app.route('/dashboard.html')
def dashboard():
    if not os.path.exists(DASHBOARD_PATH):
        return jsonify({"success": False, "error": "Không tìm thấy dashboard.html"}), 404
    return send_from_directory(PROJECT_DIR, 'dashboard.html')


# ============================================================
# API UPLOAD + CHẠY C++ + LƯU RAW CODE (DUY NHẤT)
# ============================================================

@app.route('/api/upload', methods=['POST'])
def upload_and_analyze():
    print("\n" + "=" * 70)
    print("📥 NHẬN REQUEST UPLOAD")
    print("=" * 70)

    saved_raw_codes = {} # Lưu trữ raw code: {filename: content}

    try:
        # 1. KIỂM TRA FILE
        if 'files' not in request.files:
            return jsonify({"success": False, "error": "Không có file nào được gửi"}), 400

        files = request.files.getlist('files')
        if not files:
            return jsonify({"success": False, "error": "Danh sách file rỗng"}), 400

        # 2. TẠO THƯ MỤC DATA
        os.makedirs(DATA_DIR, exist_ok=True)
        print(f"📁 DATA_DIR: {DATA_DIR}")

        # 3. LƯU FILE & ĐỌC RAW CODE
        for file in files:
            if file.filename == '':
                continue

            filename = os.path.basename(file.filename)
            file_path = os.path.join(DATA_DIR, filename)

            # --- Xử lý ZIP ---
            if filename.lower().endswith('.zip'):
                print(f"📦 Đang giải nén: {filename}")
                try:
                    with zipfile.ZipFile(file, 'r') as zip_ref:
                        zip_ref.extractall(DATA_DIR)
                    
                    # Đọc raw code từ các file .cpp bên trong ZIP
                    for info in zip_ref.infolist():
                        if not info.is_dir() and info.filename.lower().endswith('.cpp'):
                            try:
                                content = zip_ref.read(info.filename).decode('utf-8', errors='ignore')
                                saved_raw_codes[info.filename] = content
                            except Exception as e:
                                print(f"⚠️ Lỗi đọc code trong zip {info.filename}: {e}")

                except zipfile.BadZipFile:
                    return jsonify({"success": False, "error": f"File ZIP lỗi: {filename}"}), 400

            # --- Xử lý File thường (.cpp) ---
            else:
                print(f"💾 Đang lưu: {filename}")
                file.save(file_path)
                
                # Đọc Raw Code ngay sau khi lưu
                try:
                    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                        content = f.read()
                        saved_raw_codes[filename] = content
                except Exception as e:
                    print(f"⚠️ Lỗi đọc file {filename}: {e}")

        # 4. KIỂM TRA DETECTOR.EXE
        if not os.path.exists(EXE_PATH):
            return jsonify({"success": False, "error": "KHÔNG TÌM THẤY detector.exe"}), 500

        # 5. XÓA RESULTS.JSON CŨ
        if os.path.exists(RESULTS_PATH):
            try:
                os.remove(RESULTS_PATH)
            except Exception:
                pass

        # 6. CHẠY C++
        print("🚀 ĐANG CHẠY CHƯƠNG TRÌNH C++")
        
        result = subprocess.run(
            [EXE_PATH],
            cwd=BASE_DIR,
            capture_output=True,
            text=True,
            encoding='utf-8',
            errors='replace'
        )

        print(f"📝 Return code: {result.returncode}")

        if result.returncode != 0:
            print("❌ C++ BỊ LỖI")
            print(result.stderr)
            return jsonify({
                "success": False,
                "error": "Chương trình C++ bị lỗi",
                "stderr": result.stderr
            }), 500

        print("✅ C++ chạy thành công")

        # 7. ĐỌC KẾT QUẢ TỪ C++
        if not os.path.exists(RESULTS_PATH):
            return jsonify({"success": False, "error": "C++ không tạo ra results.json"}), 500

        try:
            with open(RESULTS_PATH, 'r', encoding='utf-8') as f:
                data = json.load(f)
        except json.JSONDecodeError as e:
            return jsonify({"success": False, "error": f"Lỗi JSON: {e}"}), 500

        # 8. GHÉP RAW CODE VÀO DỮ LIỆU
        if isinstance(data, dict):
            data['raw_codes'] = saved_raw_codes
        else:
            # Nếu data là list, bọc nó lại
            data = {
                "results": data,
                "raw_codes": saved_raw_codes
            }
            
        print(f"✅ Đã ghép {len(saved_raw_codes)} đoạn raw code vào kết quả.")

        # 9. LƯU LỊCH SỬ (HISTORY)
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        history_filename = f"results_{timestamp}.json"
        history_path = os.path.join(DATA_DIR, history_filename)

        with open(history_path, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=4)

        print(f"💾 Đã lưu lịch sử: {history_filename}")

        # 10. TRẢ VỀ CLIENT
        dashboard_url = f"/dashboard.html?file={history_filename}"

        return jsonify({
            "success": True,
            "filename": history_filename,
            "timestamp": timestamp,
            "data": data,
            "redirect": dashboard_url
        })

    except Exception as e:
        print("💥 LỖI HỆ THỐNG:", str(e))
        import traceback
        traceback.print_exc()
        return jsonify({"success": False, "error": f"Lỗi hệ thống: {str(e)}"}), 500


# ============================================================
# API LỊCH SỬ
# ============================================================

@app.route('/api/history', methods=['GET'])
def get_history():
    try:
        os.makedirs(DATA_DIR, exist_ok=True)
        files = [f for f in os.listdir(DATA_DIR) if f.startswith('results_') and f.endswith('.json')]
        files.sort(reverse=True) # Mới nhất trước

        history = []
        for filename in files:
            try:
                time_str = filename.replace('results_', '').replace('.json', '')
                if len(time_str) >= 15:
                    formatted_time = f"{time_str[:4]}-{time_str[4:6]}-{time_str[6:8]} {time_str[9:11]}:{time_str[11:13]}:{time_str[13:15]}"
                else:
                    formatted_time = time_str
                
                history.append({"filename": filename, "display_time": formatted_time})
            except Exception:
                continue
        return jsonify(history)
    except Exception as e:
        return jsonify({"success": False, "error": str(e)}), 500


# ============================================================
# API LẤY KẾT QUẢ
# ============================================================

@app.route('/api/results', methods=['GET'])
def get_results():
    filename = request.args.get('file')
    if not filename:
        return jsonify({"success": False, "error": "Vui lòng cung cấp tham số 'file'"}), 400

    filename = os.path.basename(filename)
    if not (filename.startswith('results_') and filename.endswith('.json')):
        return jsonify({"success": False, "error": "Tên file không hợp lệ"}), 400

    file_path = os.path.join(DATA_DIR, filename)
    if not os.path.exists(file_path):
        return jsonify({"success": False, "error": "File không tồn tại"}), 404

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        return jsonify(data)
    except Exception as e:
        return jsonify({"success": False, "error": str(e)}), 500


# ============================================================
# API OPENROUTER (AI)
# ============================================================

@app.route('/api/ai', methods=['POST'])
def ask_ai():
    print("\n" + "=" * 70)
    print("🤖 NHẬN REQUEST AI")
    print("=" * 70)

    try:
        request_data = request.get_json(silent=True)
        if not request_data:
            return jsonify({"success": False, "error": "JSON không hợp lệ"}), 400

        message = request_data.get('message', '')
        if not isinstance(message, str) or not message.strip():
            return jsonify({"success": False, "error": "Nội dung trống"}), 400

        api_key = os.environ.get('OPENROUTER_API_KEY')
        if not api_key:
            return jsonify({"success": False, "error": "Chưa cấu hình API Key"}), 500

        response = requests.post(
            "https://openrouter.ai/api/v1/chat/completions",
            headers={"Authorization": "Bearer " + api_key, "Content-Type": "application/json"},
            json={
                "model": "recursive/flint-v1.0:free",
                "messages": [{"role": "user", "content": message}]
            },
            timeout=60
        )

        if not response.ok:
            return jsonify({"success": False, "error": response.text}), response.status_code

        result = response.json()
        answer = result.get("choices", [{}])[0].get("message", {}).get("content", "")
        
        if not answer:
            return jsonify({"success": False, "error": "AI không trả về nội dung"}), 500

        return jsonify({"success": True, "answer": answer})

    except Exception as e:
        return jsonify({"success": False, "error": str(e)}), 500


# ============================================================
# API SO SÁNH RAW CODE (TEXT)
# ============================================================

@app.route('/api/compare-raw', methods=['POST'])
def compare_raw_code():
    print("\n" + "=" * 70)
    print("📝 NHẬN REQUEST SO SÁNH RAW CODE")
    print("=" * 70)

    temp_files = [] 
    try:
        request_data = request.get_json(silent=True)
        if not request_data:
            return jsonify({"success": False, "error": "JSON không hợp lệ"}), 400

        code_a = request_data.get('code_a', '')
        code_b = request_data.get('code_b', '')
        if not code_a or not code_b:
            return jsonify({"success": False, "error": "Thiếu code"}), 400

        # 1. Tạo file tạm
        timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        temp_dir = os.path.join(DATA_DIR, 'temp_compare')
        os.makedirs(temp_dir, exist_ok=True)

        file_a_name = f"temp_a_{timestamp}.cpp"
        file_b_name = f"temp_b_{timestamp}.cpp"
        path_a = os.path.join(temp_dir, file_a_name)
        path_b = os.path.join(temp_dir, file_b_name)
        temp_files.extend([path_a, path_b])

        with open(path_a, 'w', encoding='utf-8') as f: f.write(code_a)
        with open(path_b, 'w', encoding='utf-8') as f: f.write(code_b)

        # 2. Chạy C++
        if not os.path.exists(EXE_PATH):
            raise FileNotFoundError("Không tìm thấy detector.exe")

        print("🚀 Đang chạy detector.exe cho raw code...")
        # Giả định detector nhận 2 đối số file
        result = subprocess.run(
            [EXE_PATH, path_a, path_b], 
            cwd=BASE_DIR, 
            capture_output=True, text=True, encoding='utf-8', errors='replace', timeout=30
        )

        if result.returncode != 0:
            raise Exception(f"Lỗi C++: {result.stderr}")

        # 3. Đọc kết quả
        target_result_path = RESULTS_PATH 
        if not os.path.exists(target_result_path):
            temp_result = os.path.join(temp_dir, 'results.json')
            if os.path.exists(temp_result): target_result_path = temp_result
            else: raise FileNotFoundError("C++ không tạo ra kết quả")

        with open(target_result_path, 'r', encoding='utf-8') as f:
            data = json.load(f)

        return jsonify({"success": True, "data": data})

    except Exception as e:
        print("💥 LỖI:", str(e))
        return jsonify({"success": False, "error": str(e)}), 500
    
    finally:
        for f_path in temp_files:
            try:
                if os.path.exists(f_path): os.remove(f_path)
            except: pass


# ============================================================
# CHẠY SERVER
# ============================================================

if __name__ == '__main__':
    print("\n" + "=" * 70)
    print("🌐 SERVER ĐANG CHẠY")
    print("=" * 70)
    print("🔗 http://127.0.0.1:5000")
    print("📊 http://127.0.0.1:5000/dashboard.html")
    print("=" * 70)
    
    app.run(host='127.0.0.1', port=5000, debug=False)