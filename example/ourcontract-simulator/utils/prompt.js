const prompts = require('prompts');

const askConfig = async (initialFilePath) => {
	const initFilePath = initialFilePath ? { initial: initialFilePath } : {};
	const questions = [
		{
			type: 'text',
			name: 'filePath',
			message: 'where is your contract place?',
			...initFilePath,
		},
	];
	return prompts(questions);
};

module.exports = {
	askConfig,
};
