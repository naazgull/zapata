#pragma once

namespace zapata {
	// start at 1000
	enum ErrorCodeUsers {
		ERRNameMandatory = 1000,
		ERREmailMandatory = 1001,
		ERRPasswordMandatory = 1002,
		ERRConfirmationMandatory = 1003,
		ERRPasswordConfirmationDontMatch = 1004,
		ERRUserAlreadyExists = 1005
	};
}
