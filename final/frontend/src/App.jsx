import React, { useState, useEffect } from "react";
import "./styles.css";
import { fetchMessages, addMessage, fetchSummaries } from "./api";

function App() {
  const [messages, setMessages] = useState([]);
  const [summaries, setSummaries] = useState([]);
  const [content, setContent] = useState("");
  const [activeTab, setActiveTab] = useState("posts");
  const [showPostBox, setShowPostBox] = useState(false);
  const [showWelcomePopup, setShowWelcomePopup] = useState(true);
  const [openDropdowns, setOpenDropdowns] = useState({});

  useEffect(() => {
    const loadMessages = async () => {
      const msgs = await fetchMessages();
      setMessages(msgs);
    };
    const loadSummaries = async () => {
      const data = await fetchSummaries();
      setSummaries(data);
    };

    loadMessages();
    loadSummaries();
    const interval = setInterval(loadMessages, 5000);
    return () => clearInterval(interval);
  }, []);

  const handleSubmit = async (e) => {
    e.preventDefault();
    await addMessage(content);
    setContent("");
    setShowPostBox(false);
    const updated = await fetchMessages();
    setMessages(updated);
  };

  const closeWelcomePopup = () => setShowWelcomePopup(false);
  const toggleDropdown = (idx) =>
    setOpenDropdowns((prev) => ({ ...prev, [idx]: !prev[idx] }));

  return (
    <div className="container">
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
              <button className="popup-btn" onClick={closeWelcomePopup}>
                Let's Go
              </button>
            </div>
          </div>
        </div>
      )}

      <nav className="sidebar">
        <div className="logo-title">
          <img src="/llama.svg" alt="LlamaFeed Logo" className="logo" />
        </div>
        <button onClick={() => setActiveTab("posts")} className={activeTab === "posts" ? "active" : ""}>
          Home
        </button>
        <button onClick={() => setActiveTab("summaries")} className={activeTab === "summaries" ? "active" : ""}>
          Summaries
        </button>
      </nav>

      <div className="main-content">
        <div className="header">
          <h1 className="title">LlamaFeed</h1>
        </div>
        <h2 className="subtitle">{activeTab === "posts" ? "Home" : "Summaries"}</h2>

        {activeTab === "posts" && (
          <div className="posts-container">
            <div className="posts-row">
              <div className="posts-title"><h3>Posts</h3></div>
              <div className="trust-indicator-title"><h3>Trust Indicator</h3></div>
            </div>

            <div className="posts">
              {messages.map((msg, idx) => (
                <div key={idx} className="post-row">
                  <div className="post">{msg.content}</div>
                  <div className="trust-indicator">
                    <span>
                      Misinformation: {msg.is_misinformation ? "Yes ⚠️" : "No ✅"}
                      <br />
                      Offensive: {msg.is_offensive ? "Yes 🚫" : "No ✅"}
                    </span>
                    <button className="trust-dropdown" onClick={() => toggleDropdown(idx)}>
                      {openDropdowns[idx] ? "▾" : "▸"}
                    </button>
                    <div className={`dropdown-explanation ${openDropdowns[idx] ? "open" : ""}`}>
                      {msg.is_misinformation && msg.misinfo_expl && (
                        <p><strong>Misinformation →</strong> {msg.misinfo_expl}</p>
                      )}
                      {msg.is_offensive && msg.offensive_expl && (
                        <p><strong>Offensive →</strong> {msg.offensive_expl}</p>
                      )}
                      {!(msg.is_misinformation || msg.is_offensive) && (
                        <p>This post appears fine. No issues detected.</p>
                      )}
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}

        {activeTab === "summaries" && (
          <div className="summaries">
            <div className="summary-container">
              {summaries.length === 0 ? (
                <p>No summaries yet.</p>
              ) : (
                summaries.map((summary, idx) => (
                  <div key={summary.id || idx} className="summary-box">
                    <h3>Summary #{summaries.length - idx}</h3>
                    <p>{summary.content}</p>
                    <div className="summary-timestamp">
                      <small>Generated at: {new Date(summary.run_time).toLocaleString()}</small>
                    </div>
                  </div>
                ))
              )}
            </div>
          </div>
        )}
      </div>

      <button className="floating-button" onClick={() => setShowPostBox(true)}>
        +
      </button>

      {showPostBox && (
        <div className="post-box-overlay">
          <div className="post-box">
            <h3>New Post</h3>
            <div className="textarea-wrapper">
              <textarea
                value={content}
                onChange={(e) => setContent(e.target.value)}
                rows={4}
                cols={55}
                maxLength={300}
                placeholder="Type your message..."
                required
              />
              <div className={`char-counter-inside ${content.length > 250 ? "char-warning" : ""}`}>
                {content.length}/300
              </div>
            </div>
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
