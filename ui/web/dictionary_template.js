export const init_dictionary = (config) => {
    config.languages = { {{languages}} }
    config.pages.index = {
        dictionary: {
            {{field-translations}}
            {{static-translations}}
        }
    }
}
