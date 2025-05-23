from flask import Flask, jsonify, request, send_from_directory
from flask_sqlalchemy import SQLAlchemy
from flask_cors import CORS
import os
from datetime import datetime, timezone

from content_filter import check_misinformation, check_offensive

API_KEY = "123456789abcdef"

app = Flask(__name__, static_folder="../frontend/build", template_folder="../frontend/public")
CORS(app)

app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///messages.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db = SQLAlchemy(app)

class Message(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    content = db.Column(db.String(300), nullable=False)
    created_at = db.Column(db.DateTime, default=lambda: datetime.now(timezone.utc), index=True)
    is_offensive = db.Column(db.Boolean, default=False)
    is_misinformation = db.Column(db.Boolean, default=False)
    misinfo_expl = db.Column(db.Text)
    offensive_expl = db.Column(db.Text)

class DailySummary(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    run_time = db.Column(db.DateTime, default=lambda: datetime.now(timezone.utc), index=True)
    content = db.Column(db.Text, nullable=False)

with app.app_context():
    db.create_all()

@app.route("/", defaults={"path": ""})
@app.route("/<path:path>")
def serve_react(path):
    if path and os.path.exists(os.path.join(app.static_folder, path)):
        return send_from_directory(app.static_folder, path)
    return send_from_directory(app.template_folder, "index.html")

# -----------------------------------------------------------------------------
# Messages
# -----------------------------------------------------------------------------

@app.route('/api/messages', methods=['GET'])
def api_get_messages():
    messages = Message.query.order_by(Message.created_at.desc()).all()
    return jsonify([
        {
            "id": msg.id,
            "content": msg.content,
            "is_offensive": msg.is_offensive,
            "is_misinformation": msg.is_misinformation,
            "misinfo_expl": msg.misinfo_expl,
            "offensive_expl": msg.offensive_expl,
            "created_at": msg.created_at.isoformat(timespec="seconds")
        }
        for msg in messages
    ])

@app.route('/api/add_message', methods=['POST'])
def api_add_message():
    if request.headers.get('X-API-KEY') != API_KEY:
        return jsonify({'error': 'Unauthorized'}), 401

    data = request.get_json() or {}
    content = data.get('content')

    if not content or len(content) > 300:
        return jsonify({'error': 'Invalid or missing content'}), 400

    try:
        mis_flag, mis_expl = check_misinformation(content)
        off_flag, off_expl = check_offensive(content)
    except Exception as e:
        return jsonify({'error': 'LLM check failed', 'details': str(e)}), 500

    new_msg = Message(
        content=content,
        is_offensive=off_flag,
        is_misinformation=mis_flag,
        misinfo_expl=mis_expl,
        offensive_expl=off_expl
    )
    db.session.add(new_msg)
    db.session.commit()

    return jsonify({'message': 'Message added successfully', 'id': new_msg.id}), 201

# -----------------------------------------------------------------------------
# Delete older messages
# -----------------------------------------------------------------------------

@app.route('/api/delete_messages_before/<string:yyyymmdd>', methods=['DELETE'])
def api_delete_messages_before(yyyymmdd):
    if request.headers.get('X-API-KEY') != API_KEY:
        return jsonify({'error': 'Unauthorized'}), 401
    try:
        cutoff = datetime.fromisoformat(yyyymmdd)
    except ValueError:
        return jsonify({'error': 'Invalid date format; use YYYY‑MM‑DD'}), 400

    deleted = Message.query.filter(Message.created_at < cutoff).delete()
    db.session.commit()
    return jsonify({'deleted': deleted})

# -----------------------------------------------------------------------------
# Daily summaries (per script run)
# -----------------------------------------------------------------------------

@app.route('/api/daily_summary', methods=['POST'])
def api_create_summary():
    if request.headers.get('X-API-KEY') != API_KEY:
        return jsonify({'error': 'Unauthorized'}), 401

    data = request.get_json() or {}
    text = data.get("content")

    if not text:
        return jsonify({'error': 'Missing content'}), 400

    row = DailySummary(content=text)
    db.session.add(row)
    db.session.commit()

    return jsonify({"status": "ok", "id": row.id})

@app.route('/api/daily_summaries', methods=['GET'])
def api_get_summaries():
    rows = DailySummary.query.order_by(DailySummary.run_time.desc()).all()
    return jsonify([
        {
            "id": r.id,
            "content": r.content,
            "run_time": r.run_time.isoformat(timespec="seconds")
        } for r in rows
    ])

if __name__ == '__main__':
    app.run(debug=True, port=5000)
