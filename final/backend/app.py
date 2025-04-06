from flask import Flask, jsonify, request, send_from_directory
from flask_sqlalchemy import SQLAlchemy
from flask_cors import CORS
import os
from datetime import date, datetime

from content_filter import is_misinformation, is_offensive

API_KEY = "123456789abcdef"

app = Flask(__name__, static_folder="../frontend/build", template_folder="../frontend/public")
CORS(app)

app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///messages.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db = SQLAlchemy(app)

class Message(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    content = db.Column(db.String(300), nullable=False)
    is_offensive = db.Column(db.Boolean, default=False)
    is_misinformation = db.Column(db.Boolean, default=False)

class DailySummary(db.Model):
    summary_date   = db.Column(db.Date, primary_key=True, default=date.today)
    content        = db.Column(db.Text,  nullable=False)
    generated_at   = db.Column(db.DateTime, default=datetime.utcnow)

with app.app_context():
    db.create_all()

@app.route("/", defaults={"path": ""})
@app.route("/<path:path>")
def serve_react(path):
    if path and os.path.exists(os.path.join(app.static_folder, path)):
        return send_from_directory(app.static_folder, path)
    return send_from_directory(app.template_folder, "index.html")
"""
curl http://localhost:5000/api/messages
"""
@app.route('/api/messages', methods=['GET'])
def api_get_messages():
    messages = Message.query.all()
    return jsonify([
        {
            "id": msg.id,
            "content": msg.content,
            "is_offensive": msg.is_offensive,
            "is_misinformation": msg.is_misinformation
        }
        for msg in messages
    ])

"""
curl -X POST http://localhost:5000/api/add_message \
  -H "Content-Type: application/json" \
  -H "X-API-KEY: 123456789abcdef" \
  -d '{"content": "This is a test message."}'
"""
@app.route('/api/add_message', methods=['POST'])
def api_add_message():
    key = request.headers.get('X-API-KEY')
    if key != API_KEY:
        return jsonify({'error': 'Unauthorized'}), 401

    data = request.get_json()
    content = data.get('content') if data else None

    if not content or len(content) > 300:
        return jsonify({'error': 'Invalid or missing content'}), 400
    try:
        misinfo_flag = is_misinformation(content)
        offensive_flag = is_offensive(content)
    except Exception as e:
        return jsonify({'error': 'LLM check failed', 'details': str(e)}), 500

    new_msg = Message(
        content=content,
        is_offensive=offensive_flag,
        is_misinformation=misinfo_flag
    )
    db.session.add(new_msg)
    db.session.commit()

    return jsonify({'message': 'Message added successfully'})

"""
curl -X DELETE http://localhost:5000/api/delete_messages_before/2024-12-31 \
  -H "X-API-KEY: 123456789abcdef"
"""
@app.route('/api/delete_messages_before/<string:yyyymmdd>', methods=['DELETE'])
def api_delete_messages_before(yyyymmdd):
    key = request.headers.get('X-API-KEY')
    if key != API_KEY:
        return jsonify({'error': 'Unauthorized'}), 401
    try:
        cutoff = date.fromisoformat(yyyymmdd)
    except ValueError:
        return jsonify({'error': 'Invalid date format; use YYYY‑MM‑DD'}), 400

    deleted = Message.query.filter(Message.id.isnot(None),
                                   Message.created_at < cutoff).delete() \
              if hasattr(Message, "created_at") else \
              Message.query.delete()              
    db.session.commit()
    return jsonify({'deleted': deleted})

"""
curl http://localhost:5000/api/daily_summaries
"""
@app.route('/api/daily_summaries', methods=['GET'])
def api_get_summaries():
    """Return all stored summaries, newest first."""
    rows = DailySummary.query.order_by(DailySummary.summary_date.desc()).all()
    return jsonify([
        {
            "date": r.summary_date.isoformat(),
            "content": r.content,
            "generated_at": r.generated_at.isoformat(timespec="seconds")
        } for r in rows
    ])

"""
curl -X POST http://localhost:5000/api/daily_summary \
  -H "Content-Type: application/json" \
  -H "X-API-KEY: 123456789abcdef" \
  -d '{"content": "Summary for the day.", "date": "2025-04-05"}'
"""
@app.route('/api/daily_summary', methods=['POST', 'PUT'])
def api_upsert_summary():
    """Create or replace the summary for a given date (defaults to today)."""
    if request.headers.get('X-API-KEY') != API_KEY:
        return jsonify({'error': 'Unauthorized'}), 401

    data  = request.get_json() or {}
    text  = data.get("content")
    sdate = date.fromisoformat(data.get("date")) if data.get("date") else date.today()

    if not text:
        return jsonify({'error': 'Missing content'}), 400

    row = DailySummary.query.get(sdate)
    if row:                        
        row.content      = text
        row.generated_at = datetime.utcnow()
    else:                         
        row = DailySummary(summary_date=sdate, content=text)
        db.session.add(row)

    db.session.commit()
    return jsonify({"status": "ok", "date": sdate.isoformat()})


if __name__ == '__main__':
    app.run(debug=True, port=5000)
