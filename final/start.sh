#!/bin/bash

# Backend (Flask)
cd backend
source venv/bin/activate
python3 app.py &

# Frontend (React)
cd ../frontend
npm start