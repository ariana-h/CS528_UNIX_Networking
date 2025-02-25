#!/usr/bin/env python3

from ollama import Client

model_name = "llama3.2"
host_addr = "localhost"

def main():
    client = Client(host=f'http://{host_addr}:11434')
    messages = []

    while True:
        prompt = input(f"{model_name}: ")
        if prompt.lower() == 'exit':
            print("Goodbye!")
            break
    
        messages.append({'role': 'user', 'content':prompt})

        try:
            response = client.chat(model=model_name, messages=messages)
            if 'message' in response:
                message_content = response['message']['content']
                print(f"{model_name}: {message_content}")
                messages.append({'role':'assistant', 'content':message_content})
            else:
                print("Unexpected response format:", response)
        except Exception as e:
            print(f"Error: {e}")

if __name__ == "__main__":
    main()

