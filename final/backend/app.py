from flask import Flask, jsonify, request, send_from_directory
from flask_sqlalchemy import SQLAlchemy
from flask_cors import CORS
import os

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

with app.app_context():
    db.create_all()

@app.route("/", defaults={"path": ""})
@app.route("/<path:path>")
def serve_react(path):
    if path and os.path.exists(os.path.join(app.static_folder, path)):
        return send_from_directory(app.static_folder, path)
    return send_from_directory(app.template_folder, "index.html")

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

if __name__ == '__main__':
    app.run(debug=True, port=5000)
