#!/usr/bin/env python3

import threading
import time
from ollama import Client


model_1 = "llama3.2"
model_2 = "llama3.1"
host_addr = "localhost"

def create_client(name, addr):
    return Client(host=f'http://{addr}:11434')

def chat_with_model(client, model_name, messages):
    try:
        response = client.chat(model=model_name, messages=messages)
        return response['message']['content'] if "message" in response else ""
    except Exception as e: print(f"{e}")

def model_convo(client, model_name, messages):
    response = chat_with_model(client, model_name, messages)
    if response:
        print(f"[{model_name}]{response}")
        messages.append({'role': 'user', 'content': response})
        

def main():
    client = create_client(model_1, "localhost")
    host = create_client(model_2, host_addr)
    messages = []
    init_input = input(f"Initial Input: ")
    messages.append({'role': 'user', 'content': init_input})
    
    while True:
        if init_input.lower() == 'exit':
            print("Goodbye!")
            break
        
        thread1 = threading.Thread(target=model_convo, args=(client, model_1, messages))
        thread2 = threading.Thread(target=model_convo, args=(host, model_2, messages))

        thread1.start()
        thread1.join()

        thread2.start()
        thread2.join()

        #time.sleep(1)
            
if __name__ == "__main__":
    main()

