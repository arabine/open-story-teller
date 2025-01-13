import { createContext } from "preact";
import { useState } from 'preact/hooks';
import { html } from 'htm/preact';

const AppContext = createContext();

const AppProvider = ({ children }) => {
    const [state, setState] = useState({
        stories: [
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },
            { uuid: "9339d121-ea93-4cc4-9738-9979d43505d0", title: 'Lorem Ipsum', cover: "https://picsum.photos/200/300", description: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur mattis est lectus, scelerisque lobortis neque cursus id. Vestibulum id mollis magna. Etiam vitae est ut mi rutrum congue. Vestibulum a finibus orci. Nunc mattis, risus quis venenatis sagittis, nulla sem sagittis lectus, nec luctus eros neque ut lorem. Nam porttitor a augue vitae venenatis. Vivamus orci nunc, scelerisque ut lacus a, lobortis efficitur nunc. " },

        ]
    });

    const setStories = (stories) => {
        setState(prevState => ({
            ...prevState,
            stories: [...stories ],
        }));
    };

    // // Fonction pour changer les paramètres
    // const updateSettings = (newSettings) => {
    //     setState(prevState => ({
    //         ...prevState,
    //         settings: { ...prevState.settings, ...newSettings },
    //     }));
    // };

    return html`
        <${AppContext.Provider} value=${{ state, setStories }}>
            ${children}
        </${AppContext.Provider}>
    `;
};

export { AppProvider, AppContext };
