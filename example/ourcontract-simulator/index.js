const { askConfig } = require('./utils/prompt');

(async () => {
	console.log(await askConfig('AAAA'));
})();
