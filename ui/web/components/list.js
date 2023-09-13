export default {
    props: {
        lang: String,
        dictionary: Object,
        id: String,
        url: String,
        fields: Object,
        visible: Array,
        sizes: Array
    },
    data() {
        return {
            collection: [],
            received: false,
            show: 0,
            filter_key: this.visible[0],
            filter_term: null,
            sort_key: null,
            sort_order: 1,
            page_size: this.sizes[0],
            page_nr: 0,
            refresh: false
        }
    },
    computed: {
        filtered_data() {
            let sort_key = this.sort_key
            let sort_order = this.sort_order
            let filter_key = this.filter_key
            let filter_term = this.filter_term
            let page_size = this.page_size
            let page_nr = this.page_nr
            let refresh = this.refresh

            if (this.refresh) {
                this.refresh = false
            }

            if (this.received) {
                if (!this.show) {
                    this.show = 100;
                }
                this.received = false
                return this.collection
            }

            let url = this.url
            url += '?fields=' + this.visible.join(",") + this.get_url_filter_terms()
            if (filter_term) {
                url += '&' + this.filter_key + '={.like(%25' + encodeURI(filter_term) + '%25,i).}'
            }
            if (sort_key) {
                url += '&order_by=' + sort_key + '%20' + (this.sort_order == 1 ? 'asc' : 'desc')
            }
            url += '&page_size=' + page_size + '&page_start_index=' + (page_nr * page_size)

            this.fetch_data(url).then((data) => {
                this.received = true
                this.collection = data
            })
            return this.collection
        }
    },
    methods: {
        listen_to_hash() {
            if (window.location.hash == '' && window.location.hash.indexOf('?action=') == -1) {
                this.refresh = true;
            }
        },
        async fetch_data(url) {
            try {
                const reply = await fetch(url)
                if (reply.status == 200) {
                    const data = await reply.json()
                    return data.items
                }
            } catch (error) {
                console.log(error)
            }
            return []
        },
        sort_by(key) {
            if (this.sort_key != key) {
                this.sort_key = key
                this.sort_order = 1
            }
            else {
                this.sort_order *= -1
            }
        },
        prev_page() {
            if (this.page_nr > 0) {
                --this.page_nr
            }
        },
        next_page() {
            if (this.collection.length == this.page_size) {
                ++this.page_nr
            }
        },
        change_route(key, item) {
            if (this.fields[key].type != 'link') {
                window.location.hash = '/' + item
            }
        },
        get_url_filter_terms() {
            let uri = window.location.href.split('?');
            if(uri.length == 2) {
                return '&' + uri[1]
            }
            return "";
        },
        translate_value(key, record) {
            switch(this.fields[key].type) {
            case 'link':{
                let link = (record[key] ? record[key] : this.fields[key].value.href).replaceAll('{id}', record._id)
                for (let name of Object.keys(record)) {
                    link = link.replaceAll('{' + name + '}', record[key])
                }
                return '<a href="' + link + '" title="' + this.fields[key].help + '">' + this.fields[key].value.text + '</a>'
            }
            case 'dropbox':
            case 'checkbox':
            case 'radio':{
                if (this.fields[key].multiple) {
                    let values = ''
                    let first = true
                    for (let option of this.fields[key].options) {
                        if (record[key].indexOf(option.value) != -1) {
                            if (!first) {
                                values += '<br>'
                            }
                            first = false
                            values += option.text
                        }
                    }
                    return values
                }
                else {
                    for (let option of this.fields[key].options) {
                        if (option.value == record[key]) {
                            return option.text
                        }
                    }
                    return record[key];
                }
            }
            default:{
                return record[key];
            }
            }
        }
    },
    created() {
        window.addEventListener('hashchange', () => {
            this.listen_to_hash()
		    })
        this.listen_to_hash()
    },
    template: `
    <div :id="id" :style="{ opacity: show }" style="transition: opacity 0.5s ease">
      <form id="search">
          {{ dictionary.search_by[lang] }} <select name="field" v-model="filter_key">
          <option v-for="key in visible"
            :value="key">
            {{ fields[key].label }}
          </option>
        </select>: <input name="term" v-model="filter_term">
      </form>
      <table>
        <thead>
          <tr>
            <th v-for="key in visible"
              @click="sort_by(key)"
              :class="{ active: sort_key == key }">
              {{ fields[key].label }}
            </th>
          </tr>
        </thead>
        <transition-group tag="tbody" name="list-item-fade">
          <tr v-for="entry in filtered_data" v-if="filtered_data.length"
            :key="entry._id">
            <td v-for="key in visible"
              @click="change_route(key, entry._id)"
              v-html="translate_value(key, entry)"></td>
          </tr>
          <tr v-else :key="'empty_row'"><td :colspan="visible.length" style="text-align: center">--</td></tr>
        </transition-group>
      </table>
      <button @click="prev_page()">&lt;</button>
      <select name="n_elements" v-model="page_size">
        <option v-for="size in sizes" :value="size">{{ size }}</option>
      </select>
      <button @click="next_page()">&gt;</button>
    </div>
  `
}
