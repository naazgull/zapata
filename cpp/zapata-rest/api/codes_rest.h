#pragma once

namespace zapata {
	// start at 0
	enum ErrorCodeRest {
		ERRGeneric = 0,
		ERRBodyEntityMustBeProvided = 1,
		ERRBodyEntityWrongContentType = 2,
		ERRRequiredField = 3,
		ERRFileNotFound = 4,
		ERRFilePermissions = 5,
		ERRResourceNotFound = 6,
		ERRResourceIsEmpty = 7,
		ERRResourceDuplicate = 8
	};
}
