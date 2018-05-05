-- This module mainly serves as a safe way to check
-- if cLuaCov is present - the other modules are written
-- in C and may easily error on load due to a missing symbol,
-- for instance, leaving the caller of `require` thinking that
-- they are missing.
return "0.1.1"
