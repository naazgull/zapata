export const init_dictionary = (config) => {
    config.languages = { {{languages}} }
    config.pages.{{page-name}} = {
        dictionary: {
            {{field-translations}},
            {{static-translations}}
        }
    }
}
