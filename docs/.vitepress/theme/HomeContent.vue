<script setup>
import DefaultTheme from 'vitepress/theme'
import { onMounted } from 'vue'
import { ref } from 'vue'

const lastNews = ref([]);

onMounted(() => {
  fetch('https://piaille.fr/api/v1/timelines/tag/openstoryteller?limit=5').then(
    resp => resp.json() // this returns a promise
  ).then(messages => {

  //  lastNews = ref([]);
    for (const m of messages) {
      if (m.account.username === "arabine")  {
        // console.log(m)  
        console.log(m.content);
        lastNews.value.push({ content: m.content, date: new Date(m.created_at).toDateString() })
      }
    }
  }).catch(ex => {
    console.error(ex);
  })

});

</script>

<template>

<div class="vp-doc custom-block">
  <h1>Latest news</h1>
    <div v-for="item in lastNews">
        <h3>{{item.date}}</h3>
        <p v-html="item.content"></p>
    </div>

</div>

</template>

<style scoped>

</style>
