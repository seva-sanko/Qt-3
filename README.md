# Qt-3

Qt lab — phone book application (lab8).

Fields: first name, last name, patronymic, address, date of birth, email, multiple phone numbers (home/work/mobile).

All input validated with `QRegularExpression`:
- Names: letters + digits of any alphabet, hyphen allowed, must start with a capital letter, no leading/trailing hyphens
- Phone: international format (`+7 812 1234567`, `8(800)123-1212`), stored internally as digit sequence
- Date of birth: must be in the past, correct day/month ranges, leap years handled
- Email: `user@domain` format, no stray spaces

Features: add/remove/edit records, sort by any field, multi-field search, persistence via `QFile`.

Display: `QTableView` + `QTableWidget` (free column sorting).
