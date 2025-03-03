 # Ollama Conversations

## Overview
This project implements a conversation using the Ollama API. The works by creating two instances of the Llama model to communicate autonomously. The conversation can start with an initial prompt or run without direction.

## Files
- `convo.py` - The main script that sets up and runs the autonomous conversation between the two models.
- `simple.py` - An interactive version that allows human input to drive the conversation.
- `install.sh` - A setup script to configure the Ollama API and prepare the environment.
  
## Usage
### Autonomous Conversation
To run the fully autonomous conversation:
```sh
python3 convo.py
```
