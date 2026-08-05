package api

import "fmt"

type ErrorCode string

const (
	ErrorInvalidRequest    ErrorCode = "INVALID_REQUEST"
	ErrorInvalidQuery      ErrorCode = "INVALID_QUERY"
	ErrorMethodUnavailable ErrorCode = "METHOD_UNAVAILABLE"
	ErrorNotFound          ErrorCode = "NOT_FOUND"
	ErrorAmbiguousObject   ErrorCode = "AMBIGUOUS_OBJECT"
	ErrorOutsideRoot       ErrorCode = "OUTSIDE_ROOT"
	ErrorUnapprovedRoot    ErrorCode = "UNAPPROVED_ROOT"
	ErrorGenerationExpired ErrorCode = "GENERATION_EXPIRED"
	ErrorResourceBudget    ErrorCode = "RESOURCE_BUDGET_EXCEEDED"
	ErrorIntegrity         ErrorCode = "INTEGRITY_FAILURE"
	ErrorInternal          ErrorCode = "INTERNAL"
)

// Fault is safe to project across transports. Cause remains process-local so
// responses do not accidentally disclose paths or implementation details.
type Fault struct {
	Code    ErrorCode
	Message string
	Cause   error
}

func (e *Fault) Error() string {
	if e == nil {
		return "<nil>"
	}
	return fmt.Sprintf("%s: %s", e.Code, e.Message)
}

func (e *Fault) Unwrap() error { return e.Cause }

func NewFault(code ErrorCode, message string) *Fault {
	return &Fault{Code: code, Message: message}
}

func WrapFault(code ErrorCode, message string, cause error) *Fault {
	return &Fault{Code: code, Message: message, Cause: cause}
}
