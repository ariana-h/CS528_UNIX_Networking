#!/usr/bin/env python3

from ollama import Client

model_name = "llama3.2"
host_model = "llama3.1"
host_addr = "localhost"

def create_client(str):
    return Client(host=f'http://{host_addr}:11434')

def chat_with_model(client, model_name, messages):
    try:
        response = client.chat(model=model_name, messages=messages)
        if 'message' in response:
            return response['message']['content']
        else: print("Unexpected response format:", response)
    except Exception as e: print(f"Error communicating with {model_name}: {e}")

    return ""

def main():
    client = create_client(host_addr)
    host = create_client("localhost")

    messages = []
    init_input = input(f"{model_name}: ")
    while True:
        if init_input.lower() == 'exit':
            print("Goodbye!")
            break

        messages.append({'role': 'user', 'content': init_input})
        response = chat_with_model(client, model_name, messages)
        if response:
            print(f"{model_name}: {response}")
            messages.append({'role': 'assistant', 'content': response})

        response = chat_with_model(host, host_model, messages)
        if response:
            print(f"{host_model}: {response}")
            messages.append({'role': 'assistant', 'content': response})

if __name__ == "__main__":
    main()

