import React, { useState, useEffect } from "react";
import "./styles.css";
import { fetchMessages, addMessage } from "./api";

function App() {
    const [messages, setMessages] = useState([]);
    const [content, setContent] = useState("");

    useEffect(() => {
        fetchMessages().then(setMessages);
        const interval = setInterval(() => {
            fetchMessages().then(setMessages);
        }, 5000);
        return () => clearInterval(interval); // Cleanup on unmount
    }, []);

    async function handleSubmit(e) {
        e.preventDefault();
        await addMessage(content);
        setContent("");
        setMessages(await fetchMessages());
    }

    return (
        <div>
            <h1>LlamaFeed 🦙</h1>
            <form onSubmit={handleSubmit}>
                <textarea
                    value={content}
                    onChange={(e) => setContent(e.target.value)}
                    rows="4"
                    cols="50"
                    maxLength="300"
                    placeholder="Type your message..."
                    required
                />
                <br />
                <button type="submit">Submit</button>
            </form>
            <h2>Posted Messages</h2>
            <div id="messages">
                {messages.map((msg, index) => (
                    <div key={index} className="message">{msg}</div>
                ))}
            </div>
        </div>
    );
}

export default App;
