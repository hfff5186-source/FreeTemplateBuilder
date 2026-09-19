# البناء عبر GitHub Actions (من متصفح الجوال، بدون Termux)

هذي الطريقة تبني المود على سيرفرات GitHub الحقيقية (فيها NDK/Windows/macOS رسمية) وتطلع لك ملف `.geode` جاهز تحمّله من الجوال مباشرة.

## 1) أنشئ حساب GitHub (لو ما عندك)
افتح https://github.com/signup من متصفح الجوال واسجل حساب مجاني.

## 2) أنشئ Repository جديد
- من المتصفح: https://github.com/new
- اسم الريبو: `FreeTemplateBuilder` (أو أي اسم)
- خليه **Public** أو **Private** (كلاهما يشتغل مع Actions مجانًا للحسابات الشخصية)
- لا تضيف README أثناء الإنشاء (خليه فاضي)
- اضغط **Create repository**

## 3) ارفع ملفات المشروع
بصفحة الريبو الفاضي:
- اضغط **uploading an existing file**
- اسحب/اختر **كل** ملفات ومجلدات مشروع `FreeTemplateBuilder-v3` (بما فيها مجلد `.github` كامل ومجلد `src` كامل و`CMakeLists.txt` و`mod.json`)
- ⚠️ متصفحات الجوال أحيانًا ما تدعم رفع مجلد كامل دفعة وحدة. لو صار كذا:
  - ارفع الملفات المفردة (`CMakeLists.txt`, `mod.json`, `README.md`...) أول
  - بعدها لكل مجلد (`src/`, `.github/workflows/`)، اضغط **Add file → Create new file**، واكتب المسار كامل بخانة الاسم (مثلاً `src/Main.cpp`)، والصق المحتوى، واحفظ (Commit)
- بالنهاية اكتب أي رسالة بخانة **Commit changes** واضغط **Commit directly to the main branch**

## 4) شغّل البناء
- روح لتبويب **Actions** أعلى صفحة الريبو
- بيبدأ البناء تلقائيًا بعد أول رفع (لأن الـ workflow مربوط بـ `push`)
- لو ما بدأ لحاله: اضغط على workflow اسمه **Build Geode Mod** من القائمة اليسار، بعدين **Run workflow**

## 5) انتظر ثم حمّل النتيجة
- البناء ياخذ حوالي 3-8 دقائق
- ✅ أخضر = نجح، ❌ أحمر = فشل (اضغط عليه تشوف رسالة الخطأ، وابعثها لي)
- لما ينجح، افتح الـ run، انزل لقسم **Artifacts** بالأسفل، حمّل الملف اسمه `FreeTemplateBuilder-geode-package`
- فك ضغط الملف المحمّل — بداخله ملف `.geode` جاهز، انسخه لمجلد مودات Geode بجوالك

## ملاحظة مهمة
لو طلع خطأ بالبناء متعلق بأسماء الحقول (`m_editorUI`, `m_selectedObjects`, `m_objectID`...) — هذي معلّمة بتعليقات `BINDING:` بالكود، وتحتاج تعديل بسيط حسب نسخة bindings الحالية. انسخ رسالة الخطأ كاملة وابعثها هنا وأصلحها لك مباشرة.
