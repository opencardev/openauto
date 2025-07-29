# OpenAuto Modernization Action Plan
## Moving Application Logic to Backend and Implementing QT6 UI with Theme Support

### Current Architecture Analysis

The current OpenAuto project has a tightly coupled architecture where:
- **UI Layer** (`autoapp/`): Contains Qt-based UI components (MainWindow, SettingsWindow, ConnectDialog)
- **Service Layer** (`openauto/Service/`): Contains Android Auto protocol services
- **Configuration Layer** (`Configuration/`): Handles application settings
- **Business Logic**: Embedded within UI components and service layer

### Goal
Create a modern, scalable architecture with:
1. **Backend API Server**: RESTful API for all business logic
2. **Decoupled Frontend**: QT6-based UI that communicates via API
3. **Theme/Skin System**: Dynamic UI customization capabilities
4. **Service-Oriented Architecture**: Clean separation of concerns

---

## Phase 1: Backend API Infrastructure (Weeks 1-3)

### 1.1 Create API Server Foundation
```
📁 backend/
├── api/
│   ├── controllers/
│   ├── middleware/
│   ├── routes/
│   └── validators/
├── core/
│   ├── services/
│   ├── repositories/
│   └── models/
├── infrastructure/
│   ├── database/
│   ├── config/
│   └── logging/
└── main.cpp
```

**Technology Stack:**
- **HTTP Server**: cpp-httplib or Pistache for lightweight REST API
- **JSON**: nlohmann/json for request/response handling
- **Configuration**: Keep existing configuration system with API wrapper
- **Logging**: Extend existing logging system

**API Endpoints to Implement:**

#### Configuration Management
```cpp
GET    /api/v1/config                    // Get all configuration
PUT    /api/v1/config                    // Update configuration
POST   /api/v1/config/reset             // Reset to defaults
GET    /api/v1/config/schema            // Get configuration schema
```

#### Connection Management
```cpp
GET    /api/v1/connections              // Get connection status
POST   /api/v1/connections/usb          // Start USB connection
POST   /api/v1/connections/wireless     // Start wireless connection
DELETE /api/v1/connections/{id}         // Stop connection
GET    /api/v1/connections/recent       // Get recent addresses
POST   /api/v1/connections/recent       // Add recent address
```

#### Android Auto Services
```cpp
GET    /api/v1/services                 // Get service status
POST   /api/v1/services/start           // Start services
POST   /api/v1/services/stop            // Stop services
GET    /api/v1/services/video           // Video service status
GET    /api/v1/services/audio           // Audio service status
GET    /api/v1/services/input           // Input service status
```

#### System Management
```cpp
GET    /api/v1/system/status            // System status
POST   /api/v1/system/restart           // Restart system
GET    /api/v1/system/logs              // Get logs
GET    /api/v1/system/metrics           // Performance metrics
```

#### Theme Management
```cpp
GET    /api/v1/themes                   // List available themes
GET    /api/v1/themes/{id}              // Get specific theme
PUT    /api/v1/themes/{id}              // Update theme
POST   /api/v1/themes                   // Create new theme
DELETE /api/v1/themes/{id}              // Delete theme
GET    /api/v1/themes/current           // Get current theme
PUT    /api/v1/themes/current           // Set current theme
```

### 1.2 Implement Service Abstraction Layer

Create service interfaces that wrap existing OpenAuto services:

```cpp
// backend/core/services/AndroidAutoService.hpp
class AndroidAutoService {
private:
    openauto::App::Pointer app_;
    
public:
    struct ConnectionStatus {
        bool isConnected;
        std::string deviceType; // "USB" | "WIRELESS"
        std::string deviceInfo;
        std::chrono::system_clock::time_point connectedAt;
    };
    
    ConnectionStatus getStatus() const;
    void startUSBConnection();
    void startWirelessConnection(const std::string& ipAddress);
    void disconnect();
    void registerEventHandler(std::function<void(const std::string&)> handler);
};

// backend/core/services/ConfigurationService.hpp
class ConfigurationService {
private:
    openauto::configuration::IConfiguration::Pointer config_;
    
public:
    nlohmann::json getAllSettings() const;
    void updateSettings(const nlohmann::json& settings);
    void resetToDefaults();
    nlohmann::json getSchema() const;
    void save();
};

// backend/core/services/ThemeService.hpp
class ThemeService {
public:
    struct Theme {
        std::string id;
        std::string name;
        std::string description;
        nlohmann::json colorScheme;
        nlohmann::json fontSettings;
        nlohmann::json layoutSettings;
        std::string iconPack;
        std::vector<std::string> assets;
    };
    
    std::vector<Theme> getAvailableThemes() const;
    Theme getCurrentTheme() const;
    void setCurrentTheme(const std::string& themeId);
    void saveTheme(const Theme& theme);
    void deleteTheme(const std::string& themeId);
};
```

### 1.3 Configuration Migration

Extend existing `IConfiguration` interface to support JSON serialization:

```cpp
// New methods to add to IConfiguration
virtual nlohmann::json toJson() const = 0;
virtual void fromJson(const nlohmann::json& json) = 0;
virtual nlohmann::json getSchema() const = 0;
```

---

## Phase 2: Theme System Implementation (Weeks 2-4)

### 2.1 Theme Architecture

```
📁 themes/
├── default/
│   ├── theme.json
│   ├── colors.json
│   ├── fonts.json
│   ├── layouts/
│   ├── icons/
│   └── assets/
├── dark/
├── retro/
└── custom/
```

### 2.2 Theme Schema Definition

```json
{
  "theme": {
    "id": "default",
    "name": "Default Theme",
    "version": "1.0.0",
    "description": "Default OpenAuto theme",
    "colorScheme": {
      "primary": "#2196F3",
      "secondary": "#FFC107",
      "background": "#FFFFFF",
      "surface": "#F5F5F5",
      "text": "#212121",
      "textSecondary": "#757575",
      "accent": "#03DAC6",
      "error": "#F44336",
      "warning": "#FF9800",
      "success": "#4CAF50"
    },
    "typography": {
      "fontFamily": "Roboto",
      "fontSize": {
        "small": 12,
        "medium": 14,
        "large": 18,
        "xlarge": 24
      },
      "fontWeight": {
        "light": 300,
        "normal": 400,
        "bold": 700
      }
    },
    "layout": {
      "borderRadius": 8,
      "spacing": {
        "xs": 4,
        "sm": 8,
        "md": 16,
        "lg": 24,
        "xl": 32
      },
      "shadows": {
        "small": "0 2px 4px rgba(0,0,0,0.1)",
        "medium": "0 4px 8px rgba(0,0,0,0.15)",
        "large": "0 8px 16px rgba(0,0,0,0.2)"
      }
    },
    "components": {
      "button": {
        "height": 48,
        "borderRadius": 8,
        "padding": "12px 24px"
      },
      "input": {
        "height": 56,
        "borderRadius": 4,
        "padding": "16px"
      }
    },
    "iconPack": "material-design",
    "animations": {
      "duration": {
        "fast": 150,
        "normal": 300,
        "slow": 500
      },
      "easing": "ease-in-out"
    }
  }
}
```

### 2.3 Theme Management API Implementation

```cpp
// backend/api/controllers/ThemeController.cpp
class ThemeController {
public:
    void getThemes(const httplib::Request& req, httplib::Response& res);
    void getTheme(const httplib::Request& req, httplib::Response& res);
    void setCurrentTheme(const httplib::Request& req, httplib::Response& res);
    void createTheme(const httplib::Request& req, httplib::Response& res);
    void updateTheme(const httplib::Request& req, httplib::Response& res);
    void deleteTheme(const httplib::Request& req, httplib::Response& res);
};
```

---

## Phase 3: QT6 Frontend Implementation (Weeks 4-8)

### 3.1 QT6 Application Architecture

```
📁 frontend-qt6/
├── main.cpp
├── application/
│   ├── OpenAutoApp.cpp
│   └── OpenAutoApp.h
├── core/
│   ├── ApiClient.cpp
│   ├── ThemeManager.cpp
│   ├── StateManager.cpp
│   └── EventBus.cpp
├── ui/
│   ├── views/
│   │   ├── MainView.cpp
│   │   ├── SettingsView.cpp
│   │   ├── ConnectionView.cpp
│   │   └── ThemeView.cpp
│   ├── components/
│   │   ├── ThemedButton.cpp
│   │   ├── ThemedInput.cpp
│   │   ├── ThemedCard.cpp
│   │   └── ConnectionStatus.cpp
│   ├── layouts/
│   └── styles/
├── models/
│   ├── ConfigurationModel.cpp
│   ├── ConnectionModel.cpp
│   └── ThemeModel.cpp
└── resources/
    ├── qml/
    ├── images/
    └── fonts/
```

### 3.2 API Client Implementation

```cpp
// frontend-qt6/core/ApiClient.h
class ApiClient : public QObject {
    Q_OBJECT
    
public:
    explicit ApiClient(const QString& baseUrl, QObject* parent = nullptr);
    
    // Configuration methods
    QFuture<QJsonObject> getConfiguration();
    QFuture<bool> updateConfiguration(const QJsonObject& config);
    QFuture<bool> resetConfiguration();
    
    // Connection methods
    QFuture<QJsonObject> getConnectionStatus();
    QFuture<bool> startUSBConnection();
    QFuture<bool> startWirelessConnection(const QString& ipAddress);
    QFuture<bool> disconnect();
    
    // Theme methods
    QFuture<QJsonArray> getThemes();
    QFuture<QJsonObject> getCurrentTheme();
    QFuture<bool> setCurrentTheme(const QString& themeId);
    
signals:
    void connectionStatusChanged(const QJsonObject& status);
    void configurationChanged(const QJsonObject& config);
    void themeChanged(const QJsonObject& theme);
    void errorOccurred(const QString& error);
    
private:
    QNetworkAccessManager* networkManager_;
    QString baseUrl_;
    
    void setupEventStream(); // Server-Sent Events for real-time updates
};
```

### 3.3 Theme Manager Implementation

```cpp
// frontend-qt6/core/ThemeManager.h
class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QColor primaryColor READ primaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily NOTIFY themeChanged)
    Q_PROPERTY(int fontSize READ fontSize NOTIFY themeChanged)
    
public:
    explicit ThemeManager(QObject* parent = nullptr);
    
    // Theme properties
    QColor primaryColor() const;
    QColor backgroundColor() const;
    QString fontFamily() const;
    int fontSize() const;
    
    // Theme management
    void loadTheme(const QJsonObject& themeData);
    QJsonObject currentTheme() const;
    
    // Component styling
    QString buttonStyle() const;
    QString inputStyle() const;
    QString cardStyle() const;
    
public slots:
    void setTheme(const QString& themeId);
    
signals:
    void themeChanged();
    
private:
    QJsonObject currentTheme_;
    void updateStyleSheets();
};
```

### 3.4 Themed Components

```cpp
// frontend-qt6/ui/components/ThemedButton.h
class ThemedButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QString variant READ variant WRITE setVariant)
    
public:
    explicit ThemedButton(const QString& text = "", QWidget* parent = nullptr);
    
    QString variant() const { return variant_; }
    void setVariant(const QString& variant);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    
private slots:
    void updateTheme();
    
private:
    QString variant_; // "primary", "secondary", "outline", "text"
    ThemeManager* themeManager_;
};
```

### 3.5 QML Integration for Advanced Theming

```qml
// frontend-qt6/resources/qml/components/ThemedButton.qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import OpenAuto.Theme 1.0

Button {
    id: control
    
    property string variant: "primary"
    
    background: Rectangle {
        color: {
            switch(control.variant) {
                case "primary": return Theme.primaryColor
                case "secondary": return Theme.secondaryColor
                default: return Theme.surfaceColor
            }
        }
        radius: Theme.borderRadius
        border.width: control.variant === "outline" ? 1 : 0
        border.color: Theme.primaryColor
        
        Behavior on color {
            ColorAnimation { duration: Theme.animationDuration }
        }
    }
    
    contentItem: Text {
        text: control.text
        font.family: Theme.fontFamily
        font.pixelSize: Theme.fontSize
        color: control.variant === "outline" ? Theme.primaryColor : Theme.onPrimaryColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
```

---

## Phase 4: State Management & Real-time Updates (Weeks 6-8)

### 4.1 Event-Driven Architecture

```cpp
// backend/core/events/EventBus.hpp
class EventBus {
public:
    enum class EventType {
        CONNECTION_STATUS_CHANGED,
        CONFIGURATION_UPDATED,
        THEME_CHANGED,
        SERVICE_STATUS_CHANGED,
        ERROR_OCCURRED
    };
    
    struct Event {
        EventType type;
        nlohmann::json data;
        std::chrono::system_clock::time_point timestamp;
    };
    
    void publish(const Event& event);
    void subscribe(EventType type, std::function<void(const Event&)> handler);
    void unsubscribe(EventType type, const std::string& handlerId);
    
private:
    std::map<EventType, std::vector<std::pair<std::string, std::function<void(const Event&)>>>> subscribers_;
};
```

### 4.2 WebSocket/Server-Sent Events for Real-time Updates

```cpp
// backend/api/middleware/EventStreamMiddleware.hpp
class EventStreamMiddleware {
public:
    void setupEventStream(httplib::Server& server);
    void broadcastEvent(const EventBus::Event& event);
    
private:
    std::vector<httplib::Stream*> clients_;
    EventBus* eventBus_;
};
```

### 4.3 Frontend State Management

```cpp
// frontend-qt6/core/StateManager.h
class StateManager : public QObject {
    Q_OBJECT
    
public:
    explicit StateManager(ApiClient* apiClient, QObject* parent = nullptr);
    
    // State getters
    QJsonObject configuration() const;
    QJsonObject connectionStatus() const;
    QJsonObject currentTheme() const;
    QJsonArray availableThemes() const;
    
public slots:
    void refreshAll();
    void updateConfiguration(const QJsonObject& config);
    void setTheme(const QString& themeId);
    
signals:
    void stateChanged();
    void configurationChanged(const QJsonObject& config);
    void connectionStatusChanged(const QJsonObject& status);
    void themeChanged(const QJsonObject& theme);
    
private:
    ApiClient* apiClient_;
    QJsonObject state_;
    
    void handleApiEvent(const QJsonObject& event);
};
```

---

## Phase 5: Migration Strategy & Backwards Compatibility (Weeks 8-10)

### 5.1 Gradual Migration Approach

1. **Dual Operation Mode**: Run both old and new systems simultaneously
2. **Feature Flags**: Enable/disable new features progressively
3. **Configuration Migration**: Automatic migration of existing settings
4. **Fallback Mechanisms**: Graceful degradation when API is unavailable

### 5.2 Migration Scripts

```cpp
// migration/ConfigurationMigrator.cpp
class ConfigurationMigrator {
public:
    static bool migrateFromLegacy(const std::string& legacyConfigPath, 
                                 const std::string& newConfigPath);
    static bool createBackup(const std::string& configPath);
    static bool rollback(const std::string& backupPath, 
                        const std::string& configPath);
};
```

### 5.3 Compatibility Layer

```cpp
// compatibility/LegacyAdapter.hpp
class LegacyAdapter {
public:
    // Wrap legacy UI components for gradual migration
    static QWidget* wrapLegacyWindow(QWidget* legacyWindow);
    
    // Bridge legacy configuration system
    static void syncLegacyConfig(openauto::configuration::IConfiguration* legacy,
                               ConfigurationService* modern);
};
```

---

## Phase 6: Advanced Features & Polish (Weeks 10-12)

### 6.1 Advanced Theme Features

1. **Dynamic Theme Switching**: Hot-swappable themes without restart
2. **Theme Editor**: Visual theme customization interface
3. **Adaptive Themes**: Automatic light/dark mode based on time/ambient light
4. **Custom Components**: User-defined UI components
5. **Animation System**: Smooth transitions and micro-interactions

### 6.2 Plugin Architecture

```cpp
// plugins/IUIPlugin.hpp
class IUIPlugin {
public:
    virtual ~IUIPlugin() = default;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QWidget* createWidget(QWidget* parent = nullptr) = 0;
    virtual void initialize(ApiClient* apiClient) = 0;
    virtual void cleanup() = 0;
};
```

### 6.3 Performance Optimizations

1. **Lazy Loading**: Load UI components on demand
2. **Caching**: Cache API responses and theme assets
3. **Resource Management**: Efficient memory and GPU usage
4. **Startup Optimization**: Fast application boot times

---

## Implementation Timeline

| Phase | Duration | Key Deliverables |
|-------|----------|-----------------|
| **Phase 1** | Weeks 1-3 | Backend API Server, Service Abstraction |
| **Phase 2** | Weeks 2-4 | Theme System, Theme Management API |
| **Phase 3** | Weeks 4-8 | QT6 Frontend, Themed Components |
| **Phase 4** | Weeks 6-8 | Real-time Updates, State Management |
| **Phase 5** | Weeks 8-10 | Migration Tools, Compatibility Layer |
| **Phase 6** | Weeks 10-12 | Advanced Features, Performance Optimization |

## File Structure Overview

```
📁 openauto-modernized/
├── backend/                     # New API backend
│   ├── api/
│   ├── core/
│   ├── infrastructure/
│   └── main.cpp
├── frontend-qt6/                # New QT6 frontend
│   ├── core/
│   ├── ui/
│   ├── models/
│   └── main.cpp
├── themes/                      # Theme system
│   ├── default/
│   ├── dark/
│   └── custom/
├── plugins/                     # Plugin system
├── migration/                   # Migration tools
├── compatibility/               # Legacy compatibility
├── openauto/                    # Original backend (legacy)
├── autoapp/                     # Original frontend (legacy)
└── docs/                        # Documentation
```

## Benefits of This Architecture

1. **Separation of Concerns**: Clear boundaries between UI, business logic, and data
2. **Scalability**: Easy to add new features and UI components
3. **Maintainability**: Modular code structure with clear interfaces
4. **Flexibility**: Theme system allows easy customization
5. **Modern Technology**: QT6 with latest features and performance improvements
6. **API-First**: Enables future mobile apps, web interfaces, or third-party integrations
7. **Real-time Updates**: Responsive UI with immediate feedback
8. **Plugin Support**: Extensible architecture for custom features

## Next Steps

1. **Setup Development Environment**: Configure QT6, HTTP library, and build system
2. **Create Project Structure**: Initialize the new directory structure
3. **Implement API Foundation**: Start with basic HTTP server and configuration endpoints
4. **Build Theme System**: Create theme schema and management system
5. **Develop QT6 Components**: Begin with basic themed components
6. **Integration Testing**: Ensure backend and frontend communicate properly
7. **Migration Planning**: Prepare migration scripts and compatibility layer
8. **Documentation**: Create developer and user documentation
9. **Testing Strategy**: Unit tests, integration tests, and user acceptance testing
10. **Deployment**: Production deployment and monitoring setup

This action plan provides a comprehensive roadmap for modernizing OpenAuto with a clear separation between backend business logic and a themeable QT6 frontend, enabling easier maintenance, customization, and future enhancements.
