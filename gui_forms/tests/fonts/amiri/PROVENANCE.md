# Arabic shaping test fixture

Unmodified `Amiri-Regular.ttf` and its `OFL.txt` from Google Fonts:
<https://github.com/google/fonts/tree/fffdadf0f0c9cc1ec8b407063424a8bfbee05611/ofl/amiri>.
The font is 431,116 bytes; copyright belongs to the Amiri Project Authors.
The accompanying SIL Open Font License 1.1 is retained in full.

SHA-256 of downloaded bytes:

- `Amiri-Regular.ttf`: `ab391c4147d054c48976e98322ad0eefe1427aa0e0502a12a4c75d80a70cfcd7`
- `OFL.txt`: `72de68e5954f4fdd24702292ef5a32f003ca960ec9330dc86e5eefb5dffb9b22`

**OBSERVED coordinator intake:** accepted as an explicitly registered test bank
for positive Arabic joining/contextual-shaping evidence. The existing Carlito
and Cousine fixtures lack the required Arabic coverage. This test-only file is
not a host-font lookup, application default/fallback replacement, installed SDK
asset or adoption of Games' fonts. Its presence alone proves no shaping result;
the provider must check coverage and exercise contextual behavior. No font bytes
were modified, subset or renamed internally.

The upstream license retains its original whitespace; Git's whitespace checker
reports its trailing space/blank line. Those vendor bytes are intentionally
preserved, and the local attributes disable newline conversion for its hash.
