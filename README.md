# ft_irc - IRC Server Implementation

## English | العربية

A complete IRC (Internet Relay Chat) server implementation in C++ 98 for the 42 School project.

خادم IRC كامل تم تنفيذه باستخدام C++ 98 لمشروع 42.

---

## Features | المميزات

### English:
- **Multi-client support** using non-blocking I/O and `poll()`
- **Non-blocking operations only** - no forking
- **Single `poll()` call** for all I/O operations
- **Basic IRC commands**: PASS, NICK, USER, JOIN, PART, PRIVMSG, QUIT
- **Channel operations**: JOIN, PART, KICK, INVITE, TOPIC, MODE
- **Channel modes**: `i` (invite-only), `t` (topic restricted), `k` (key), `o` (operator), `l` (limit)
- **Proper error handling** with RFC 2812 compliant responses
- **Partial message handling** to support fragmented data
- **Memory leak-free** implementation

### العربية:
- **دعم عملاء متعددين** باستخدام I/O غير حظر و`poll()`
- **عمليات غير حظر فقط** - بدون forking
- **استدعاء `poll()` واحد** لجميع عمليات I/O
- **أوامر IRC الأساسية**: PASS, NICK, USER, JOIN, PART, PRIVMSG, QUIT
- **عمليات القنوات**: JOIN, PART, KICK, INVITE, TOPIC, MODE
- **أوضاع القنوات**: `i` (دعوة فقط)، `t` (موضوع محجوب)، `k` (مفتاح)، `o` (مشرف)، `l` (حد)
- **معالجة أخطاء صحيحة** مع استجابات متوافقة مع RFC 2812
- **التعامل مع الرسائل الجزئية** لدعم البيانات المجزأة
- **تنفيذ بدون تسرب للذاكرة**

---

## Requirements | المتطلبات

### English:
- **Language**: C++ 98 standard
- **Compiler**: clang++ or g++
- **Operating System**: Unix-based (Linux, macOS)
- **External Libraries**: None (only standard socket functions)

### العربية:
- **اللغة**: معيار C++ 98
- **المترجم**: clang++ أو g++
- **نظام التشغيل**: يعتمد على Unix (Linux، macOS)
- **المكتبات الخارجية**: لا يوجد (دوال socket القياسية فقط)

---

## Project Structure | هيكل المشروع

```
ft_irc_server/
├── includes/          # Header files
│   ├── Server.hpp     # Main server class
│   ├── Client.hpp     # Client class
│   ├── Channel.hpp    # Channel class
│   ├── Message.hpp    # Message parser
│   ├── Commands.hpp   # Command declarations
│   └── Utils.hpp      # Utility functions
├── sources/           # Source files
│   ├── main.cpp       # Entry point
│   ├── Server.cpp     # Server implementation
│   ├── Client.cpp     # Client implementation
│   ├── Channel.cpp    # Channel implementation
│   ├── Message.cpp    # Message parser implementation
│   ├── Commands.cpp   # Command implementations
│   └── Utils.cpp      # Utility functions implementation
├── scripts/           # Test scripts
│   ├── test_basic_commands.sh
│   ├── test_channel_operations.sh
│   ├── test_operator_commands.sh
│   ├── test_stress.sh
│   └── final_test.sh
├── Makefile           # Build configuration
└── README.md          # This file
```

---

## Building | التجميع

### English:
```bash
# Clean build
make clean

# Compile the project
make

# Force rebuild
make re
```

### العربية:
```bash
# تنظيف البناء
make clean

# تجميع المشروع
make

# إعادة البناء بالقوة
make re
```

---

## Usage | الاستخدام

### English:
```bash
./ircserv <port> <password>
```

**Example:**
```bash
./ircserv 6667 mypassword
```

### العربية:
```bash
./ircserv <port> <password>
```

**مثال:**
```bash
./ircserv 6667 كلمة_المرور
```

---

## Connecting to the Server | الاتصال بالخادم

### English:

#### Using netcat:
```bash
nc -C localhost 6667
```

#### Using IRC client (irssi):
```bash
irssi -c localhost -p 6667 -w mypassword
```

#### Using IRC client (HexChat):
- Server: `localhost`
- Port: `6667`
- Password: `mypassword`

#### Manual connection sequence:
```
PASS mypassword
NICK mynickname
USER myusername 0 * :My Real Name
JOIN #testchannel
PRIVMSG #testchannel :Hello everyone!
```

### العربية:

#### باستخدام netcat:
```bash
nc -C localhost 6667
```

#### باستخدام عميل IRC (irssi):
```bash
irssi -c localhost -p 6667 -w كلمة_المرور
```

#### باستخدام عميل IRC (HexChat):
- الخادم: `localhost`
- المنفذ: `6667`
- كلمة المرور: `mypassword`

#### تسلسل الاتصال اليدوي:
```
PASS كلمة_المرور
NICK الاسم_المستعار
USER اسم_المستخدم 0 * :الاسم_الحقيقي
JOIN #قناة_الاختبار
PRIVMSG #قناة_الاختبار :السلام عليكم!
```

---

## Implemented Commands | الأوامر المنفذة

### Authentication Commands | أوامر المصادقة

#### PASS
**English**: Set the connection password
**العربية**: تعيين كلمة مرور الاتصال
```
PASS <password>
```

#### NICK
**English**: Set or change your nickname
**العربية**: تعيين أو تغيير اسمك المستعار
```
NICK <nickname>
```

#### USER
**English**: Set your username and realname
**العربية**: تعيين اسم المستخدم والاسم الحقيقي
```
USER <username> <mode> <unused> :<realname>
```

### Channel Commands | أوامر القنوات

#### JOIN
**English**: Join one or more channels
**العربية**: الانضمام لقناة أو أكثر
```
JOIN <channel>[,<channel>...] [<key>[,<key>...]]
```

#### PART
**English**: Leave one or more channels
**العربية**: مغادرة قناة أو أكثر
```
PART <channel>[,<channel>...] [<reason>]
```

#### PRIVMSG
**English**: Send a private message to user or channel
**العربية**: إرسال رسالة خاصة لمستخدم أو قناة
```
PRIVMSG <target> :<message>
```

#### QUIT
**English**: Disconnect from the server
**العربية**: قطع الاتصال من الخادم
```
QUIT [<reason>]
```

### Operator Commands | أوامر المشرفين

#### KICK
**English**: Remove a user from a channel
**العربية**: إزالة مستخدم من قناة
```
KICK <channel> <user> [:<comment>]
```

#### INVITE
**English**: Invite a user to a channel
**العربية**: دعوة مستخدم لقناة
```
INVITE <nickname> <channel>
```

#### TOPIC
**English**: View or change the channel topic
**العربية**: عرض أو تغيير موضوع القناة
```
TOPIC <channel> [:<topic>]
```

#### MODE
**English**: View or change channel modes
**العربية**: عرض أو تغيير أوضاع القناة
```
MODE <channel> [<modes> [<mode parameters>]]
```

**Available modes | الأوضاع المتاحة:**
- `+i` / `-i`: Invite-only / الدعوة فقط
- `+t` / `-t`: Topic restricted / موضوع محجوب
- `+k <key>` / `-k`: Set/remove key / تعيين/إزالة المفتاح
- `+o <nick>` / `-o <nick>`: Give/take operator / إعطاء/أخذ صلاحية المشرف
- `+l <limit>` / `-l`: Set/remove user limit / تعيين/إزالة حد المستخدمين

---

## Testing | الاختبار

### English:
All test scripts are located in the `scripts/` directory:

```bash
# Make all scripts executable
chmod +x scripts/*.sh

# Run basic commands test
./scripts/test_basic_commands.sh

# Run channel operations test
./scripts/test_channel_operations.sh

# Run operator commands test
./scripts/test_operator_commands.sh

# Run stress test (20 clients)
./scripts/test_stress.sh

# Run complete final test suite
./scripts/final_test.sh
```

### Partial Message Test:
To test partial message handling:
```bash
nc -C localhost 6667
# Type: PASS testpass, press Ctrl+D
# Type: mypass, press Ctrl+D
# Type: word, press Enter
```

### Memory Leak Check:
```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 testpass
```

### العربية:
جميع سكربتات الاختبار موجودة في مجلد `scripts/`:

```bash
# جعل جميع السكربتات قابلة للتنفيذ
chmod +x scripts/*.sh

# تشغيل اختبار الأوامر الأساسية
./scripts/test_basic_commands.sh

# تشغيل اختبار عمليات القنوات
./scripts/test_channel_operations.sh

# تشغيل اختبار أوامر المشرفين
./scripts/test_operator_commands.sh

# تشغيل اختبار الإجهاد (20 عميل)
./scripts/test_stress.sh

# تشغيل مجموعة الاختبار النهائية الكاملة
./scripts/final_test.sh
```

### اختبار الرسائل الجزئية:
لاختبار التعامل مع الرسائل الجزئية:
```bash
nc -C localhost 6667
# اكتب: PASS testpass، اضغط Ctrl+D
# اكتب: mypass، اضغط Ctrl+D
# اكتب: word، اضغط Enter
```

### فحص تسرب الذاكرة:
```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 testpass
```

---

## Error Codes | رموز الخطأ

### English:
The server implements standard IRC error codes:

- `401`: No such nick/channel
- `403`: No such channel
- `404`: Cannot send to channel
- `411`: No recipient given (PRIVMSG)
- `412`: No text to send
- `421`: Unknown command
- `431`: No nickname given
- `432`: Erroneus nickname
- `433`: Nickname already in use
- `441`: User not in channel
- `442`: You're not on that channel
- `443`: User already in channel
- `461`: Not enough parameters
- `462`: Already registered
- `464`: Password incorrect
- `471`: Cannot join (channel full)
- `473`: Cannot join (invite only)
- `475`: Cannot join (wrong key)
- `482`: Not channel operator

### العربية:
يقوم الخادم بتنفيذ رموز خطأ IRC القياسية:

- `401`: لا يوجد اسم مستعار/قناة
- `403`: لا توجد قناة
- `404`: لا يمكن الإرسال للقناة
- `411`: لم يتم تحديد مستلم (PRIVMSG)
- `412`: لا يوجد نص للإرسال
- `421`: أمر غير معروف
- `431`: لم يتم تحديد اسم مستعار
- `432`: اسم مستعار خاطئ
- `433`: اسم المستعار مستخدم بالفعل
- `441`: المستخدم ليس في القناة
- `442`: لست في تلك القناة
- `443`: المستخدم موجود بالفعل في القناة
- `461`: معلمات غير كافية
- `462`: مسجل بالفعل
- `464`: كلمة المرور غير صحيحة
- `471`: لا يمكن الانضمام (القناة ممتلئة)
- `473`: لا يمكن الانضمام (دعوة فقط)
- `475`: لا يمكن الانضمام (مفتاح خاطئ)
- `482`: لست مشرف القناة

---

## Technical Details | التفاصيل التقنية

### English:
- **Socket Management**: Non-blocking sockets with `fcntl()`
- **Event Loop**: Single `poll()` call handles all I/O events
- **Message Parsing**: RFC 2812 compliant message format
- **Connection Handling**: Each client has its own buffer for partial messages
- **Channel Management**: Dynamic channel creation and deletion
- **Operator System**: First user in channel becomes operator by default

### العربية:
- **إدارة Socket**: sockets غير حظر مع `fcntl()`
- **حلقة الأحداث**: استدعاء `poll()` واحد يعالج جميع أحداث I/O
- **تحليل الرسائل**: صيغة رسائل متوافقة مع RFC 2812
- **التعامل مع الاتصالات**: لكل عميل مخزن خاص بالرسائل الجزئية
- **إدارة القنوات**: إنشاء وحذف القنوات ديناميكياً
- **نظام المشرفين**: المستخدم الأول في القناة يصبح مشرفاً افتراضياً

---

## Compliance | الامتثال

### English:
This implementation complies with:
- **RFC 2812**: Internet Relay Chat: Client Protocol
- **42 School ft_irc project requirements**
- **C++ 98 standard**
- **Non-blocking I/O with single `poll()`**

### العربية:
يتوافق هذا التنفيذ مع:
- **RFC 2812**: محادثة IRC: بروتوكول العميل
- **متطلبات مشروع ft_irc في مدرسة 42**
- **معيار C++ 98**
- **I/O غير حظر مع `poll()` واحد**

---

## License | الترخيص

### English:
This project is part of the 42 School curriculum.

### العربية:
هذا المشروع جزء من المنهج الدراسي لمدرسة 42.

---

## Authors | المؤلفون

### English:
- **Team Member A**: Network layer, Server core, Message parsing, Basic commands
- **Team Member B**: Data structures, Channel operations, Operator commands, Testing

### العربية:
- **عضو الفريق A**: طبقة الشبكة، الخادم الأساسي، تحليل الرسائل، الأوامر الأساسية
- **عضو الفريق B**: هياكل البيانات، عمليات القنوات، أوامر المشرفين، الاختبار

---

## References | المراجع

### English:
- [RFC 2812 - IRC Client Protocol](https://tools.ietf.org/html/rfc2812)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [IRC Protocol Documentation](https://irc-wiki.org/IRC_Protocol)

### العربية:
- [RFC 2812 - بروتوكول عميل IRC](https://tools.ietf.org/html/rfc2812)
- [دليل Beej لبرمجة الشبكات](https://beej.us/guide/bgnet/)
- [توثيق بروتوكول IRC](https://irc-wiki.org/IRC_Protocol)

---

**Good luck with your evaluation! | حظاً موفقاً في تقييمك!**
