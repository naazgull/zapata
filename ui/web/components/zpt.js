function zpt_get_lang() {
}

export default {
    get_lang: function () {
        let language = localStorage.getItem('lang')
        let browserLang = navigator.language || navigator.userLanguage;
        return language != undefined ? language : browserLang.split('-')[0]
    },
    set_lang: function (language) {
        localStorage.setItem('lang', language)
    },
    menu: { items: [] },
    pages: {}
}
