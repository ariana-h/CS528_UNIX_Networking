import React, { useState, useEffect } from "react";
import "./styles.css";
import { fetchMessages, addMessage } from "./api";

function App() {
    const [messages, setMessages] = useState([]);
    const [content, setContent] = useState("");
    const [activeTab, setActiveTab] = useState("posts");
    const [trustLevels, setTrustLevels] = useState({});
    const [showPostBox, setShowPostBox] = useState(false);

    useEffect(() => {
        fetchMessages().then((msgs) => {
            setMessages(msgs);
            setTrustLevels(generateTrustLevels(msgs));
        });
        const interval = setInterval(() => {
            fetchMessages().then((msgs) => {
                setMessages(msgs);
                setTrustLevels(generateTrustLevels(msgs));
            });
        }, 5000);
        return () => clearInterval(interval);
    }, []);

    async function handleSubmit(e) {
        e.preventDefault();
        await addMessage(content);
        setContent("");
        setShowPostBox(false);
        const updatedMessages = await fetchMessages();
        setMessages(updatedMessages);
        setTrustLevels(generateTrustLevels(updatedMessages));
    }

    function generateTrustLevels(messages) {
        return messages.reduce((acc, msg, index) => {
            acc[index] = Math.random() > 0.5 ? "Likely True" : "Potentially False";
            return acc;
        }, {});
    }

    return (
        <div className="container">
            {/* Sidebar Navigation */}
            <nav className="sidebar">
                <div className="logo-title">
                    <img src="/llama.svg" alt="LlamaFeed Logo" className="logo" />
                </div>
                <button onClick={() => setActiveTab("posts")} className={activeTab === "posts" ? "active" : ""}>Home</button>
                <button onClick={() => setActiveTab("summaries")} className={activeTab === "summaries" ? "active" : ""}>Summaries</button>
            </nav>

            {/* Main Content */}
            <div className="main-content">
                <div className="header">
                    <h1 className="title">LlamaFeed</h1>
                </div>
                {activeTab === "posts" && <h2 className="subtitle">Home</h2>}
                {activeTab === "summaries" && <h2 className="subtitle">Summaries</h2>}

                {activeTab === "posts" && (
                    <div className="posts-container">
                        <div className="posts-row">
                            {/* Posts Subtitle */}
                            <div className="posts-title">
                                <h3>Posts</h3>
                            </div>
                            {/* Trust Indicator Subtitle */}
                            <div className="trust-indicator-title">
                                <h3>Trust Indicator</h3>
                            </div>
                        </div>    
                        <div className="posts">
                            {messages.map((msg, index) => (
                                <div key={index} className="post-row">
                                    <div className="post">{msg}</div>
                                    <div className="trust-indicator">
                                        <span>{trustLevels[index]}</span>
                                        <button className="trust-dropdown" onClick={() => alert('Dropdown clicked!')}>
                                            ⬇
                                        </button>
                                    </div>
                                </div>
                            ))}
                        </div>
                    </div>
                )}

                {activeTab === "summaries" && (
                    <div className="summaries">
                        <p>Summarized content will appear here.</p>
                    </div>
                )}
            </div>

            {/* Floating Button to Open Post Box */}
            <button className="floating-button" onClick={() => setShowPostBox(true)}>+</button>

            {/* Popup Post Box */}
            {showPostBox && (
                <div className="post-box-overlay">
                    <div className="post-box">
                        <h3>New Post</h3>
                        <textarea
                            value={content}
                            onChange={(e) => setContent(e.target.value)}
                            rows="4"
                            cols="50"
                            maxLength="300"
                            placeholder="Type your message..."
                            required
                        />
                        <div className="post-box-buttons">
                            <button onClick={() => setShowPostBox(false)}>Cancel</button>
                            <button onClick={handleSubmit}>Submit</button>
                        </div>
                    </div>
                </div>
            )}
        </div>
    );
}

export default App;