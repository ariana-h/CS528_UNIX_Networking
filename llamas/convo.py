#!/usr/bin/env python3

import threading
from ollama import Client


model_1 = "llama3.2"
model_2 = "llama3.1"
host_addr = "150.156.81.69"

messages_1 = []
messages_2 = []

def create_client(addr):
    return Client(host=f'http://{addr}:11434')

def chat_with_model(client, model_name, messages):
    try:
        response = client.chat(model=model_name, messages=messages)
        return response['message']['content'] if "message" in response else ""
    except Exception as e: print(f"{e}")

def model_convo(client, model_name, messages, alt_messages):
    response = chat_with_model(client, model_name, messages)
    if response:
        print(f"[{model_name}]{response}")
        messages.append({'role': 'assistant', 'content': response})
        alt_messages.append({'role': 'user', 'content': response})
        

def main():
    client = create_client("localhost")
    host = create_client(host_addr)
    
    init_input = input(f"Initial Input: ")
    messages_1.append({'role': 'user', 'content': init_input})

    while True:
        if init_input.lower() == 'exit':
            print("Goodbye!")
            break
        
        thread1 = threading.Thread(target=model_convo, args=(client, model_1, messages_1, messages_2))
        thread2 = threading.Thread(target=model_convo, args=(host, model_2, messages_2, messages_1))

        thread1.start()
        thread1.join()

        thread2.start()
        thread2.join()
            
if __name__ == "__main__":
    main()

