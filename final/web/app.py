from flask import Flask, render_template, request, redirect, jsonify
from flask_sqlalchemy import SQLAlchemy


# yes yes.. i know
API_KEY = "123456789abcdef"

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///messages.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

db = SQLAlchemy(app)

# --- Message Model ---
class Message(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    content = db.Column(db.String(300), nullable=False)
    is_offensive = db.Column(db.Boolean, default=False)
    is_misinformation = db.Column(db.Boolean, default=False)

# --- Create DB Tables on Startup ---
with app.app_context():
    db.create_all()

# --- Main Route ---
@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        content = request.form['content']
        if content:
            new_msg = Message(content=content)
            db.session.add(new_msg)
            db.session.commit()
            return redirect('/')
    messages = Message.query.all()
    return render_template('index.html', messages=messages)

@app.route('/api/add_message', methods=['POST'])
def api_add_message():
    key = request.headers.get('X-API-KEY')
    if key != API_KEY:
        return jsonify({'error': 'Unauthorized'}), 401

    data = request.get_json()
    content = data.get('content') if data else None

    if not content or len(content) > 300:
        return jsonify({'error': 'Invalid or missing content'}), 400

    new_msg = Message(content=content)
    db.session.add(new_msg)
    db.session.commit()

    return jsonify({'message': 'Message added successfully'})

@app.route('/api/messages', methods=['GET'])
def api_get_messages():
    messages = Message.query.all()
    return jsonify([msg.content for msg in messages])

if __name__ == '__main__':
    app.run(debug=True)

