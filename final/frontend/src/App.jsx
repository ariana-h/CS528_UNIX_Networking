import React, { useState, useEffect } from "react";
import "./styles.css";
import { fetchMessages, addMessage } from "./api";

function App() {
    const [messages, setMessages] = useState([]);
    const [content, setContent] = useState("");
    const [activeTab, setActiveTab] = useState("posts");
    const [trustLevels, setTrustLevels] = useState({});
    const [showPostBox, setShowPostBox] = useState(false);
    const [showWelcomePopup, setShowWelcomePopup] = useState(true);
    const [openDropdowns, setOpenDropdowns] = useState({});

    useEffect(() => {
        // Fetch messages on mount and at regular intervals
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

    const closeWelcomePopup = () => {
        setShowWelcomePopup(false);
    };

    const toggleDropdown = (index) => {
        setOpenDropdowns((prev) => ({
            ...prev,
            [index]: !prev[index],
        }));
    };

    return (
        <div className="container">
            {/* Welcome Popup */}
            {showWelcomePopup && (
                <div className="popup-overlay">
                    <div className="popup-content">
                        <div className="popup-header">
                            <h1 className="popup-title">
                                Welcome to <img src="/llama.svg" alt="Llama Logo" className="popup-logo" /> LlamaFeed!
                            </h1>
                        </div>
                        <p className="popup-subtitle">no drama, just llama.</p>
                        <div className="popup-btn-container">
                            <button className="popup-btn" onClick={closeWelcomePopup}>Let's Go</button>
                        </div>
                    </div>
                </div>
            )}

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
                                    <div className="post">{msg.content}</div>
                                    <div className="trust-indicator">
                                        <span>
                                            Misinformation: {msg.is_misinformation ? "Yes ⚠️" : "No ✅"}<br />
                                            Offensive: {msg.is_offensive ? "Yes 🚫" : "No ✅ "}
                                        </span>
                                        <button className="trust-dropdown" onClick={() => toggleDropdown(index)}>
                                            {openDropdowns[index] ? "▾" : "▸"}
                                        </button>
                                        {openDropdowns[index] && (
                                            <div className="dropdown-explanation">
                                                <p>
                                                    This post was analyzed using a language model classifier. A "Yes" means the post has signs
                                                    of misinformation or offensive content. Use your judgment when reading.
                                                </p>
                                            </div>
                                        )}
                                    </div>
                                </div>
                            ))}
                        </div>
                    </div>
                )}

                {activeTab === "summaries" && (
                    <div className="summaries">
                        <div className="summary-container">
                            <div className="summary-box">
                                <h3>Day 1</h3>
                                <p>This is an empty summary box. You can add your content here.</p>
                            </div>
                        </div>
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
                            cols="55"
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
