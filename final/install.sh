#!/bin/bash

# Backend (Flask)
cd backend
python3 -m venv venv
source venv/bin/activate
pip install flask flask_sqlalchemy
pip install flask-cors
cd ..

if ! command -v npm &> /dev/null; then
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        sudo apt update && sudo apt install -y nodejs npm
    fi
fi

# Frontend (React)
cd frontend
npm install
cd ..

