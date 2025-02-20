#!/bin/python3

import requests
import json
import os

model_name = "llama3.2"
host_addr = "localhost" #"150.156.81.69"
def main():
    url = f"http://{host_addr}:11434/api/generate"

    while True:
        prompt = input(f"{model_name}: ")
        if prompt.lower() == 'exit':
            print("Goodbye!")
            break

        payload = {
            "model": model_name,
            "prompt": prompt
        }

        try:
            with requests.post(url, json=payload, stream=True) as response:
                response.raise_for_status()
                
                complete_response = ""
                for line in response.iter_lines():
                    if line:
                        try:
                            data = json.loads(line.decode('utf-8'))
                            if "response" in data:
                                complete_response += data["response"]
                            if data.get("done", False):
                                break
                        except json.JSONDecodeError:
                            print("Non-JSON Response:", line.decode('utf-8'))
                
                print(f"{model_name}: ", complete_response)
        except requests.exceptions.RequestException as e:
            print(f"Error: {e}")

if __name__ == "__main__":
    main()

